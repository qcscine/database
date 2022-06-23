/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include "Helpers.h"
#include <Database/Collection.h>
#include <Database/Layout.h>
#include <Database/Manager.h>
#include <Database/Objects/ElementaryStep.h>
#include <Database/Objects/Reaction.h>
#include <Database/Objects/ReactionSide.h>
#include <Database/Objects/Structure.h>
#include <Utils/Math/BSplines/ReactionProfileInterpolation.h>
#include <Utils/Pybind.h>

using namespace Scine::Database;

void init_elementary_step(pybind11::module& m) {
  pybind11::class_<ElementaryStep, Object> elementaryStep(m, "ElementaryStep",
                                                          R"delim(
      Two sets of structures connected by a transition state

      An elementary step has the following relationships to other database
      objects:
      - ``Structure``: An elementary step connects two sets of structures, one
        set on each side of the transition state
      - ``Reaction``: A reaction groups elementary steps, abstracting away
        conformational differences between structures.

      Data-wise, the elementary step consists of a transition state
      ``Structure``, two sets of ``Structures``, one on each ``Side`` of the
      transition state, and a linked ``Reaction``.

      :example:
      >>> lhs = [ID(), ID()]  # Generate a few placeholder IDs
      >>> rhs = [ID()]
      >>> collection = manager.get_collection("elementary_steps")
      >>> step = ElementaryStep.make(lhs, rhs, collection)
      >>> step.transition_state_id = ID()
      >>> step.reaction_id = ID()
      >>> reactants_tup = step.get_reactants(Side.BOTH)
      >>> reactants_tup == (lhs, rhs)
      True
      >>> step.reactants_counts
      (2, 1)
      >>> step.has_reaction()
      True
    )delim");
  elementaryStep.def(pybind11::init<>());
  elementaryStep.def(pybind11::init<const ID&>());
  elementaryStep.def(pybind11::init<const ID&, const Object::CollectionPtr&>());

  elementaryStep.def_static("make",
                            pybind11::overload_cast<const std::vector<ID>&, const std::vector<ID>&, const Object::CollectionPtr&>(
                                &ElementaryStep::create),
                            pybind11::arg("lhs"), pybind11::arg("rhs"), pybind11::arg("collection"));
  elementaryStep.def("create",
                     pybind11::overload_cast<const std::vector<ID>&, const std::vector<ID>&>(&ElementaryStep::create),
                     pybind11::arg("lhs"), pybind11::arg("rhs"));

  elementaryStep.def("get_reaction", pybind11::overload_cast<>(&ElementaryStep::getReaction, pybind11::const_));
  elementaryStep.def("set_reaction", &ElementaryStep::setReaction, pybind11::arg("reaction_id"));
  elementaryStep.def("has_reaction", &ElementaryStep::hasReaction);
  elementaryStep.def("clear_reaction", &ElementaryStep::clearReaction);
  def_optional_property<ElementaryStep>(
      elementaryStep, "reaction_id", std::mem_fn(&ElementaryStep::hasReaction),
      [](const ElementaryStep& s) { return s.getReaction(); }, std::mem_fn(&ElementaryStep::setReaction),
      std::mem_fn(&ElementaryStep::clearReaction), "The reaction this elementary step belongs to");
  elementaryStep.def("get_reaction",
                     pybind11::overload_cast<const Manager&, const std::string&>(&ElementaryStep::getReaction, pybind11::const_),
                     pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::reaction,
                     "Fetch the linked reaction instance");

  elementaryStep.def("get_transition_state", pybind11::overload_cast<>(&ElementaryStep::getTransitionState, pybind11::const_));
  elementaryStep.def("set_transition_state", &ElementaryStep::setTransitionState, pybind11::arg("transition_state_id"));
  elementaryStep.def("has_transition_state", &ElementaryStep::hasTransitionState);
  elementaryStep.def("clear_transition_state", &ElementaryStep::clearTransitionState);
  def_optional_property<ElementaryStep>(
      elementaryStep, "transition_state_id", std::mem_fn(&ElementaryStep::hasTransitionState),
      [](const ElementaryStep& s) { return s.getTransitionState(); }, std::mem_fn(&ElementaryStep::setTransitionState),
      std::mem_fn(&ElementaryStep::clearTransitionState), "The transition state structure ID");
  elementaryStep.def("get_transition_state",
                     pybind11::overload_cast<const Manager&, const std::string&>(&ElementaryStep::getTransitionState,
                                                                                 pybind11::const_),
                     pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::structure,
                     "Fetch the linked transition state structure");

  elementaryStep.def("has_reactant", &ElementaryStep::hasReactant, pybind11::arg("id"));
  elementaryStep.def("add_reactant", &ElementaryStep::addReactant, pybind11::arg("id"), pybind11::arg("side"));
  elementaryStep.def("remove_reactant", &ElementaryStep::removeReactant, pybind11::arg("id"), pybind11::arg("side"));
  elementaryStep.def("has_reactants", &ElementaryStep::hasReactants);
  elementaryStep.def_property_readonly("reactants_counts", &ElementaryStep::hasReactants);
  elementaryStep.def("set_reactants", &ElementaryStep::setReactants, pybind11::arg("ids"), pybind11::arg("side"));
  elementaryStep.def("get_reactants", &ElementaryStep::getReactants, pybind11::arg("side") = SIDE::BOTH);
  elementaryStep.def("clear_reactants", &ElementaryStep::clearReactants, pybind11::arg("side"));
  elementaryStep.def("has_spline", &ElementaryStep::hasSpline);
  elementaryStep.def("set_spline", &ElementaryStep::setSpline, pybind11::arg("spline"));
  elementaryStep.def("get_spline", &ElementaryStep::getSpline);
  elementaryStep.def("clear_spline", &ElementaryStep::clearSpline);

  pybind11::enum_<ElementaryStepType> estype(m, "ElementaryStepType");
  estype.value("REGULAR", ElementaryStepType::REGULAR, "A regular elementary step that involves exactly one transition state.");
  estype.value("BARRIERLESS", ElementaryStepType::BARRIERLESS, "An elementary step that has no barrier/ transition state");
  elementaryStep.def("get_type", &ElementaryStep::getType);
  elementaryStep.def("set_type", &ElementaryStep::setType);
}
