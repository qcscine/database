/**
 * @file Misc.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_MISC_H_
#define DATABASE_MISC_H_

#include <cstring>
#include <string>
#include <vector>

namespace Scine {
namespace Database {
namespace Misc {

/**
 * @brief Computes the Levenshtein distance function for two strings.
 *
 * Nomenclature and theory were taken from:
 * https://en.wikipedia.org/wiki/Levenshtein_distance (accessed 05.04.2019)
 * Code was adapted from:
 * https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance (accessed 05.04.2019)
 *
 * @param a The first string.
 * @param b The second string.
 * @param insertCost The cost (distance) added for each insertion needed.
 * @param deleteCost The cost (distance) added for each deletion needed.
 * @param replaceCost The cost (distance) added for each replacement needed.
 * @return unsigned int The distance.
 */
static unsigned int levenshtein(const char* a, const char* b, unsigned int insertCost = 1, unsigned int deleteCost = 1,
                                unsigned int replaceCost = 1) {
  const unsigned int sizea = strlen(a);
  const unsigned int sizeb = strlen(b);
  if (sizea > sizeb) {
    return levenshtein(b, a, deleteCost, insertCost, replaceCost);
  }
  std::vector<unsigned int> distances(sizea + 1);
  distances[0] = 0;
  for (unsigned int i = 1; i <= sizea; ++i) {
    distances[i] = distances[i - 1] + deleteCost;
  }
  for (unsigned int j = 1; j <= sizeb; ++j) {
    unsigned int previousDiagonal = distances[0];
    unsigned int previousDiagonalSave;
    distances[0] += insertCost;
    for (unsigned int i = 1; i <= sizea; ++i) {
      previousDiagonalSave = distances[i];
      if (a[i - 1] == b[j - 1]) {
        distances[i] = previousDiagonal;
      }
      else {
        distances[i] =
            std::min(std::min(distances[i - 1] + deleteCost, distances[i] + insertCost), previousDiagonal + replaceCost);
      }
      previousDiagonal = previousDiagonalSave;
    }
  }
  return distances[sizea];
}
/**
 * @brief Computes the Levenshtein distance function for two strings.
 *
 * @param a The first string.
 * @param b The second string.
 * @param insertCost The cost (distance) added for each insertion needed.
 * @param deleteCost The cost (distance) added for each deletion needed.
 * @param replaceCost The cost (distance) added for each replacement needed.
 * @return unsigned int The distance.
 */
static unsigned int levenshtein(std::string a, std::string b, unsigned int insertCost = 1, unsigned int deleteCost = 1,
                                unsigned int replaceCost = 1) {
  return levenshtein(a.c_str(), b.c_str(), deleteCost, insertCost, replaceCost);
}

} // namespace Misc
} // namespace Database
} // namespace Scine

#endif // DATABASE_MISC_H_
