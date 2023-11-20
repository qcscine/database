/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include "Helpers.h"
#include <Database/Collection.h>
#include <Database/Manager.h>
#include <Database/Objects/Compound.h>
#include <Database/Objects/ElementaryStep.h>
#include <Database/Objects/Reaction.h>
#include <Utils/Pybind.h>

using namespace Scine::Database;

void init_reaction(pybind11::class_<Reaction, Object>& reaction) {
  reaction.doc() = R"delim(
    Grouping of ElementarySteps that operate on common Compounds

    A reaction is a set of elementary steps, all connecting structures from
    the same compounds in the same way.

    A reaction has the following relationships with other database objects:
    - ``Compound``: A reaction groups elementary steps that operate on the same
      compounds.
    - ``ElementaryStep``: A reaction is a set of elementary steps.

    Data-wise, a reaction consists of two sets of compounds and a set of
    elementary steps. Each structure in the elementary steps should be part of
    the compounds.

    :example:
    >>> lhs_structures = [ID(), ID()]
    >>> rhs_structures = [ID()]
    >>> steps = manager.get_collection("elementary_steps")
    >>> step = ElementaryStep.make(lhs_structures, rhs_structures, steps)
    >>> lhs_compounds = [ID(), ID()]
    >>> rhs_compounds = [ID()]
    >>> reactions = manager.get_collection("reactions")
    >>> reaction = Reaction.make(lhs_compounds, rhs_compounds, reactions)
    >>> reaction.elementary_step_ids = [step.id()]
    >>> reaction.get_reactants(Side.LHS) == (lhs_compounds, [])
    True
    >>> reaction.get_reactants(Side.RHS) == ([], rhs_compounds)
    True
  )delim";
  reaction.def(pybind11::init<>());
  reaction.def(pybind11::init<const ID&>());
  reaction.def(pybind11::init<const ID&, const Object::CollectionPtr&>());

  reaction.def_static(
      "make",
      pybind11::overload_cast<const std::vector<ID>&, const std::vector<ID>&, const Object::CollectionPtr&,
                              const std::vector<COMPOUND_OR_FLASK>&, const std::vector<COMPOUND_OR_FLASK>&>(&Reaction::create),
      pybind11::arg("lhs"), pybind11::arg("rhs"), pybind11::arg("collection"),
      pybind11::arg("lhsTypes") = std::vector<COMPOUND_OR_FLASK>({}),
      pybind11::arg("rhsTypes") = std::vector<COMPOUND_OR_FLASK>({}));
  reaction.def("create",
               pybind11::overload_cast<const std::vector<ID>&, const std::vector<ID>&, const std::vector<COMPOUND_OR_FLASK>&,
                                       const std::vector<COMPOUND_OR_FLASK>&>(&Reaction::create),
               pybind11::arg("lhs"), pybind11::arg("rhs"), pybind11::arg("lhsTypes") = std::vector<COMPOUND_OR_FLASK>({}),
               pybind11::arg("rhsTypes") = std::vector<COMPOUND_OR_FLASK>({}));

  reaction.def("has_elementary_step", &Reaction::hasElementaryStep, pybind11::arg("id"));
  reaction.def("add_elementary_step", &Reaction::addElementaryStep, pybind11::arg("id"));
  reaction.def("remove_elementary_step", &Reaction::removeElementaryStep, pybind11::arg("id"));

  reaction.def("has_elementary_steps", &Reaction::hasElementarySteps);
  reaction.def("set_elementary_steps", &Reaction::setElementarySteps, pybind11::arg("ids"));
  reaction.def("get_elementary_steps", pybind11::overload_cast<>(&Reaction::getElementarySteps, pybind11::const_));
  reaction.def("clear_elementary_steps", &Reaction::clearElementarySteps);
  def_optional_property<Reaction>(
      reaction, "elementary_step_ids", std::mem_fn(&Reaction::hasElementarySteps),
      [](const Reaction& r) { return r.getElementarySteps(); }, std::mem_fn(&Reaction::setElementarySteps),
      std::mem_fn(&Reaction::clearElementarySteps), "Linked elementary step ids");
  reaction.def("get_elementary_steps",
               pybind11::overload_cast<const Manager&, const std::string&>(&Reaction::getElementarySteps, pybind11::const_),
               pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::elementaryStep,
               "Fetch all elementary steps constituting the reaction");

  reaction.def("has_reactant", &Reaction::hasReactant, pybind11::arg("id"));
  reaction.def("add_reactant", &Reaction::addReactant, pybind11::arg("id"), pybind11::arg("side"), pybind11::arg("type"));
  reaction.def("remove_reactant", &Reaction::removeReactant, pybind11::arg("id"), pybind11::arg("side"));
  reaction.def("has_reactants", &Reaction::hasReactants);
  reaction.def("set_reactants", &Reaction::setReactants, pybind11::arg("ids"), pybind11::arg("side"),
               pybind11::arg("types") = std::vector<COMPOUND_OR_FLASK>({}));
  reaction.def("get_reactants", pybind11::overload_cast<const SIDE>(&Reaction::getReactants, pybind11::const_),
               pybind11::arg("side"));
  reaction.def("get_reactant_types", &Reaction::getReactantTypes, pybind11::arg("side"));
  reaction.def("get_reactant_type", &Reaction::getReactantType, pybind11::arg("ids"));
  reaction.def("clear_reactants", &Reaction::clearReactants, pybind11::arg("side"));
}
