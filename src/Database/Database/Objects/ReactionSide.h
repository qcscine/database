/**
 * @file ReactionSide.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_REACTIONSIDE_H_
#define DATABASE_REACTIONSIDE_H_

namespace Scine {
namespace Database {

/**
 * @brief Labels for the two sides of a Reaction or Elementary step.
 */
enum class SIDE { NONE = 0, LHS = 1, RHS = 2, BOTH = 3 };

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_REACTIONSIDE_H_ */
