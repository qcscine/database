/**
 * @file ReactionEnums.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include "Database/Objects/ReactionEnums.h"

namespace Scine {
namespace Database {

// clang-format off
const std::map<std::string, COMPOUND_OR_FLASK> EnumMaps::str2reactant = {
  {"compound", COMPOUND_OR_FLASK::COMPOUND},
  {"flask", COMPOUND_OR_FLASK::FLASK},
};
const std::map<COMPOUND_OR_FLASK, std::string> EnumMaps::reactant2str = {
  {COMPOUND_OR_FLASK::COMPOUND, "compound"},
  {COMPOUND_OR_FLASK::FLASK, "flask"}
};
// clang-format on

} /* namespace Database */
} /* namespace Scine */
