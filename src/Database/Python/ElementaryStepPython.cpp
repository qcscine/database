/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include "Helpers.h"
#include <Database/Collection.h>
#include <Database/Layout.h>
#include <Database/Manager.h>
#include <Database/Objects/ElementaryStep.h>
#include <Database/Objects/Reaction.h>
#include <Database/Objects/ReactionEnums.h>
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
      The `idx_map` arrays can be used to store which atom indices of the
      (joined) structures on one side of the elementary step correspond to which
      atom indices of the joined structures on the other side or of the
      transition state.

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
      >>> step.add_idx_maps([1, 2, 0, 3]) # Add the lhs-rhs map
      >>> step.has_idx_map(ElementaryStep.IdxMapType.LHS_RHS)
      True
      >>> step.has_idx_map(ElementaryStep.IdxMapType.LHS_TS)
      False
      >>> step.add_idx_maps([1, 2, 0, 3], [1, 3, 0, 2]) # Add the lhs-rhs and lhs-ts map
      >>> step.has_idx_map(ElementaryStep.IdxMapType.LHS_TS)
      True
      >>> step.has_idx_map(ElementaryStep.IdxMapType.TS_RHS) # ts-rhs from combination of stored maps
      True
      >>> step.get_idx_map(ElementaryStep.IdxMapType.TS_RHS)
      [0, 1, 3, 2]
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

  elementaryStep.def("has_structure_in_path", &ElementaryStep::hasStructureInPath, pybind11::arg("id"));
  elementaryStep.def("get_path", pybind11::overload_cast<>(&ElementaryStep::getPath, pybind11::const_));
  elementaryStep.def("get_path",
                     pybind11::overload_cast<const Manager&, const std::string&>(&ElementaryStep::getPath, pybind11::const_),
                     pybind11::arg("manager"), pybind11::arg("collection") = Layout::DefaultCollection::structure,
                     "Fetch all structures belonging to the elementary step's path");
  elementaryStep.def("has_path", &ElementaryStep::hasPath);
  elementaryStep.def("set_path", &ElementaryStep::setPath, pybind11::arg("ids"));
  elementaryStep.def("clear_path", &ElementaryStep::clearPath);

  pybind11::enum_<ElementaryStepType> estype(m, "ElementaryStepType");
  estype.value("REGULAR", ElementaryStepType::REGULAR, "A regular elementary step that involves exactly one transition state.");
  estype.value("BARRIERLESS", ElementaryStepType::BARRIERLESS, "An elementary step that has no barrier/ transition state");
  elementaryStep.def("get_type", &ElementaryStep::getType);
  elementaryStep.def("set_type", &ElementaryStep::setType);
  elementaryStep.def("get_barrier_from_spline", &ElementaryStep::getBarrierFromSpline, R"delim(
      Returns the rhs and lhs barrier calculated from the spline as a tuple. If no spline is available, (0.0, 0.0)
      is returned.)delim");

  pybind11::enum_<ElementaryStep::IdxMapType> idxMapType(elementaryStep, "IdxMapType");
  idxMapType.value("LHS_TS", ElementaryStep::IdxMapType::LHS_TS, "The atoms index map from lhs to ts.");
  idxMapType.value("LHS_RHS", ElementaryStep::IdxMapType::LHS_RHS, "The atoms index map from lhs to rhs.");
  idxMapType.value("TS_LHS", ElementaryStep::IdxMapType::TS_LHS, "The atoms index map from ts to lhs.");
  idxMapType.value("RHS_LHS", ElementaryStep::IdxMapType::RHS_LHS, "The atoms index map from rhs to lhs.");
  idxMapType.value("TS_RHS", ElementaryStep::IdxMapType::TS_RHS, "The atoms index map from ts to rhs.");
  idxMapType.value("RHS_TS", ElementaryStep::IdxMapType::RHS_TS, "The atoms index map from rhs to ts.");
  elementaryStep.def("add_idx_maps", &ElementaryStep::addIdxMaps, pybind11::arg("lhs_rhs_map"),
                     pybind11::arg("lhs_ts_map") = pybind11::none(),
                     "Adds the atom index map(s). The lhs to ts map is optional.");
  elementaryStep.def("remove_idx_maps", &ElementaryStep::removeIdxMaps, "Removes the atom index maps.");
  elementaryStep.def("has_idx_map", &ElementaryStep::hasIdxMap, pybind11::arg("map_type"),
                     "Checks whether a map with the given type exists or can be retrieved from the existing ones.");
  elementaryStep.def("get_idx_map", &ElementaryStep::getIdxMap, pybind11::arg("map_type"),
                     "Gets the atom index map of the given type.");
}
