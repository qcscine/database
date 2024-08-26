/**
 * @file Version.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_VERSION_H_
#define DATABASE_VERSION_H_

namespace Scine {
namespace Database {
/**
 * @brief This version refers to the database layout not the version of the
 *        wrapper code.
 */
namespace Version {

constexpr static const int major = 1;
constexpr static const int minor = 3;
constexpr static const int patch = 0;

} // namespace Version
} // namespace Database
} // namespace Scine

#endif // DATABASE_VERSION_H_
