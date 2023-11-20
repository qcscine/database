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
#include <Database/Objects/Flask.h>
#include <Database/Objects/Reaction.h>
#include <Database/Objects/Structure.h>
#include <Utils/Pybind.h>

using namespace Scine::Database;

void init_flask(pybind11::class_<Flask, Object>& flask) {
  flask.def(pybind11::init<>());
  flask.def(pybind11::init<const ID&>(), pybind11::arg("id"),
            R"delim(
      Create a new flask with a database ID. A Flask in this state is not
      linked to any collection and most interface functions will raise Errors.
    )delim");
  flask.def(pybind11::init<const ID&, const Object::CollectionPtr&>(), pybind11::arg("id"), pybind11::arg("collection"));

  flask.def_static(
      "make",
      pybind11::overload_cast<const std::vector<ID>&, const std::vector<ID>&, const Object::CollectionPtr&>(&Flask::create),
      pybind11::arg("structure_ids"), pybind11::arg("compound_ids"), pybind11::arg("collection"),
      R"delim(
      Create a new flask with a database ID.

      :returns: A Flask object linked to the given collection.
    )delim");

  flask.def("create", pybind11::overload_cast<const std::vector<ID>&, const std::vector<ID>&>(&Flask::create),
            pybind11::arg("structure_ids"), pybind11::arg("compound_ids"),
            R"delim(
      Generates a new flask in the linked collection from a list of
      structure IDs and compoud IDs. Stores the generated ID in the
      Flask object.

      :raises RuntimeError: If no collection is linked.

      :returns: The ID of the generated flask.
    )delim");

  flask.def("get_centroid", pybind11::overload_cast<>(&Flask::getCentroid, pybind11::const_),
            R"delim(
      Returns the first entry in the vector of structures

      :raises RuntimeError: If no collection is linked.
    )delim");

  flask.def("get_centroid", pybind11::overload_cast<const Manager&, const std::string&>(&Flask::getCentroid, pybind11::const_),
            pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::structure,
            "Fetch the first entry in the list of structures belonging to the flask");

  flask.def("has_reaction", &Flask::hasReaction, pybind11::arg("id"),
            R"delim(
      Checks if the flask is part of a given reaction by its ID.

      :raises RuntimeError: If no collection is linked.
    )delim");
  flask.def("add_reaction", &Flask::addReaction, pybind11::arg("id"));
  flask.def("remove_reaction", &Flask::removeReaction, pybind11::arg("id"));
  flask.def("has_reactions", &Flask::hasReactions);
  flask.def("get_reactions", pybind11::overload_cast<>(&Flask::getReactions, pybind11::const_));
  flask.def("set_reactions", &Flask::setReactions, pybind11::arg("ids"));
  flask.def("clear_reactions", &Flask::clearReactions);
  flask.def("clear_structures", &Flask::clearStructures);
  flask.def("get_reactions",
            pybind11::overload_cast<const Manager&, const std::string&>(&Flask::getReactions, pybind11::const_),
            pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::reaction,
            "Fetch all reactions known for the flask");

  flask.def("has_structure", &Flask::hasStructure, pybind11::arg("id"));
  flask.def("add_structure", &Flask::addStructure, pybind11::arg("id"));
  flask.def("remove_structure", &Flask::removeStructure, pybind11::arg("id"));
  flask.def("has_structures", &Flask::hasStructures);
  flask.def("get_structures", pybind11::overload_cast<>(&Flask::getStructures, pybind11::const_));
  flask.def("set_structures", &Flask::setStructures, pybind11::arg("ids"));
  flask.def("clear_structures", &Flask::clearStructures);
  flask.def("get_structures",
            pybind11::overload_cast<const Manager&, const std::string&>(&Flask::getStructures, pybind11::const_),
            pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::structure,
            "Fetch all structures belonging to the flask");
  flask.def("has_compound", &Flask::hasCompound, pybind11::arg("id"));
  flask.def("has_compounds", &Flask::hasCompounds);
  flask.def("get_compounds", pybind11::overload_cast<>(&Flask::getCompounds, pybind11::const_));
  flask.def("set_compounds", &Flask::setCompounds, pybind11::arg("ids"));
  flask.def("clear_compounds", &Flask::clearCompounds);
  flask.def("get_compounds",
            pybind11::overload_cast<const Manager&, const std::string&>(&Flask::getCompounds, pybind11::const_),
            pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::compound,
            "Fetch all compounds belonging to the flask");
}
