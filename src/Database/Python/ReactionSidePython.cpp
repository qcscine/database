/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include <Database/Objects/ReactionSide.h>
#include <Utils/Pybind.h>

using namespace Scine::Database;

void init_reaction_side(pybind11::module& m) {
  pybind11::enum_<SIDE> label(m, "Side");
  label.value("NONE", SIDE::NONE);
  label.value("LHS", SIDE::LHS);
  label.value("RHS", SIDE::RHS);
  label.value("BOTH", SIDE::BOTH);
}
