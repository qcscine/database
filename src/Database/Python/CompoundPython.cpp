/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include <Database/Collection.h>
#include <Database/Layout.h>
#include <Database/Manager.h>
#include <Database/Objects/Compound.h>
#include <Database/Objects/Reaction.h>
#include <Database/Objects/Structure.h>
#include <Utils/Pybind.h>

using namespace Scine::Database;

void init_compound(pybind11::class_<Compound, Object>& compound) {
  compound.def(pybind11::init<>());
  compound.def(pybind11::init<const ID&>(), pybind11::arg("id"),
               R"delim(
      Create a new compound with a database ID. A Compound in this state is not
      linked to any collection and most interface functions will raise Errors.
    )delim");
  compound.def(pybind11::init<const ID&, const Object::CollectionPtr&>(), pybind11::arg("id"), pybind11::arg("collection"));

  compound.def_static(
      "make", pybind11::overload_cast<const std::vector<ID>&, const Object::CollectionPtr&, bool>(&Compound::create),
      pybind11::arg("structure_ids"), pybind11::arg("collection"), pybind11::arg("exploration_disabled") = false);

  compound.def("create", pybind11::overload_cast<const std::vector<ID>&, bool>(&Compound::create),
               pybind11::arg("structure_ids"), pybind11::arg("exploration_disabled") = false,
               R"delim(
      Generates a new compound in the linked collection from a list of
      structure IDs.

      :raises RuntimeError: If no collection is linked.

      :returns: The ID of the generated compound.
    )delim");

  compound.def("get_centroid", pybind11::overload_cast<>(&Compound::getCentroid, pybind11::const_),
               R"delim(
      Returns the first entry in the vector of structures

      :raises RuntimeError: If no collection is linked.
    )delim");

  compound.def("get_centroid",
               pybind11::overload_cast<const Manager&, const std::string&>(&Compound::getCentroid, pybind11::const_),
               pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::structure,
               "Fetch the first entry in the list of structures belonging to the compound");

  compound.def("has_reaction", &Compound::hasReaction, pybind11::arg("id"),
               R"delim(
      Checks if the compound is part of a given reaction by its ID.

      :raises RuntimeError: If no collection is linked.
    )delim");
  compound.def("add_reaction", &Compound::addReaction, pybind11::arg("id"));
  compound.def("remove_reaction", &Compound::removeReaction, pybind11::arg("id"));
  compound.def("has_reactions", &Compound::hasReactions);
  compound.def("get_reactions", pybind11::overload_cast<>(&Compound::getReactions, pybind11::const_));
  compound.def("set_reactions", &Compound::setReactions, pybind11::arg("ids"));
  compound.def("clear_reactions", &Compound::clearReactions);
  compound.def("clear_structures", &Compound::clearStructures);
  compound.def("get_reactions",
               pybind11::overload_cast<const Manager&, const std::string&>(&Compound::getReactions, pybind11::const_),
               pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::reaction,
               "Fetch all reactions known for the compound");

  compound.def("has_structure", &Compound::hasStructure, pybind11::arg("id"));
  compound.def("add_structure", &Compound::addStructure, pybind11::arg("id"));
  compound.def("remove_structure", &Compound::removeStructure, pybind11::arg("id"));
  compound.def("has_structures", &Compound::hasStructures);
  compound.def("get_structures", pybind11::overload_cast<>(&Compound::getStructures, pybind11::const_));
  compound.def("set_structures", &Compound::setStructures, pybind11::arg("ids"));
  compound.def("clear_structures", &Compound::clearStructures);
  compound.def("get_structures",
               pybind11::overload_cast<const Manager&, const std::string&>(&Compound::getStructures, pybind11::const_),
               pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::structure,
               "Fetch all structures belonging to the compound");
}
