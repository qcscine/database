/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include "Database/Misc.h"
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {

/**
 * @class LevenshteinTest LevenshteinTest.cpp
 * @brief Comprises tests for the class Scine::Database::Misc::levenshtein.
 * @test
 */
class LevenshteinTest : public Test {};

TEST_F(LevenshteinTest, StringVersion) {
  std::string a = "kitten";
  std::string b = "sitting";
  EXPECT_EQ(Misc::levenshtein(a, b), 3);
}

TEST_F(LevenshteinTest, ConsCharPtrVersion) {
  const char* a = "kitten";
  const char* b = "sitting";
  EXPECT_EQ(Misc::levenshtein(a, b), 3);
}

TEST_F(LevenshteinTest, Results) {
  EXPECT_EQ(Misc::levenshtein("kitten", "sitting"), 3);
  EXPECT_EQ(Misc::levenshtein("a", "b"), 1);
  EXPECT_EQ(Misc::levenshtein("ab", "cd"), 2);
  EXPECT_EQ(Misc::levenshtein("ab", "cb"), 1);
}

TEST_F(LevenshteinTest, NonDefaultCosts) {
  EXPECT_EQ(Misc::levenshtein("kitten", "sitting"), 3);
  EXPECT_EQ(Misc::levenshtein("a", "b", 1, 1, 2), 2);
  EXPECT_EQ(Misc::levenshtein("ab", "cde", 2, 1, 1), 4);
  EXPECT_EQ(Misc::levenshtein("ab", "cde", 1, 2, 1), 3);
  EXPECT_EQ(Misc::levenshtein("abe", "cb", 1, 2, 1), 3);
  EXPECT_EQ(Misc::levenshtein("abe", "cb", 2, 1, 1), 2);
}

} // namespace Tests
} // namespace Database
} // namespace Scine
