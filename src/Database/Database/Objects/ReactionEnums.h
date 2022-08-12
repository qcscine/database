/**
 * @file ReactionEnums.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_REACTIONENUMS_H_
#define DATABASE_REACTIONENUMS_H_

#include <map>
#include <string>

namespace Scine {
namespace Database {

/**
 * @brief Labels for the two sides of a Reaction or Elementary step.
 */
enum class SIDE { NONE = 0, LHS = 1, RHS = 2, BOTH = 3 };

/**
 * @brief Labels tracking the type of reagents on either side of the reaction.
 */
enum class COMPOUND_OR_FLASK { COMPOUND = 0, FLASK = 1 };

struct EnumMaps {
  /// @brief maps reactant type database strings onto enum members
  static const std::map<std::string, COMPOUND_OR_FLASK> str2reactant;
  /// @brief maps reactant type enum members onto database strings
  static const std::map<COMPOUND_OR_FLASK, std::string> reactant2str;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_REACTIONENUMS_H_ */
