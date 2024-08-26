/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef INCLUDE_SCINE_DATABASE_LAYOUT_H
#define INCLUDE_SCINE_DATABASE_LAYOUT_H

#include <array>
#include <map>
#include <string>

namespace Scine {
namespace Database {

enum class CalculationStatus : unsigned {
  CONSTRUCTION = 0,
  NEW = 1,
  PENDING = 2,
  COMPLETE = 3,
  ANALYZED = 4,
  HOLD = 10,
  FAILED = 99
};

enum class StructureLabel : unsigned {
  NONE = 0,
  USER_GUESS = 1,
  USER_OPTIMIZED = 2,
  MINIMUM_GUESS = 3,
  MINIMUM_OPTIMIZED = 4,
  TS_GUESS = 5,
  TS_OPTIMIZED = 6,
  ELEMENTARY_STEP_GUESS = 7,
  ELEMENTARY_STEP_OPTIMIZED = 8,
  REACTIVE_COMPLEX_GUESS = 9,
  REACTIVE_COMPLEX_SCANNED = 10,
  REACTIVE_COMPLEX_OPTIMIZED = 11,
  SURFACE_GUESS = 12,
  SURFACE_OPTIMIZED = 13,
  SURFACE_ADSORPTION_GUESS = 14,
  COMPLEX_GUESS = 15,
  COMPLEX_OPTIMIZED = 16,
  SURFACE_COMPLEX_OPTIMIZED = 17,
  USER_SURFACE_OPTIMIZED = 18,
  USER_COMPLEX_OPTIMIZED = 19,
  USER_SURFACE_COMPLEX_OPTIMIZED = 20,
  IRRELEVANT = 99,
  DUPLICATE = 100,
  GEOMETRY_OPTIMIZATION_OBSERVER = 101,
  TS_OPTIMIZATION_OBSERVER = 102,
  IRC_FORWARD_OBSERVER = 103,
  IRC_BACKWARD_OBSERVER = 104,
  IRC_OPT_FORWARD_OBSERVER = 105,
  IRC_OPT_BACKWARD_OBSERVER = 106,
  SCAN_OBSERVER = 107,
};

enum class ElementaryStepType : unsigned { REGULAR = 0, BARRIERLESS = 1 };

namespace Layout {

struct DefaultCollection {
  constexpr static const char* structure = "structures";
  constexpr static const char* calculation = "calculations";
  constexpr static const char* elementaryStep = "elementary_steps";
  constexpr static const char* property = "properties";
  constexpr static const char* reaction = "reactions";
  constexpr static const char* compound = "compounds";
  constexpr static const char* flask = "flasks";

  constexpr static std::array<const char*, 7> all{{structure, calculation, elementaryStep, property, reaction, compound, flask}};
};

struct InternalCollection {
  constexpr static const char* meta = "_db_meta_data";
};

struct EnumMaps {
  /// @brief maps status strings onto enum members
  static const std::map<std::string, CalculationStatus> str2status;
  /// @brief maps status enum members onto strings
  static const std::map<CalculationStatus, std::string> status2str;

  /// @brief maps elementary step type strings onto enum members
  static const std::map<std::string, ElementaryStepType> str2estype;
  /// @brief maps elementary step type enum members onto strings
  static const std::map<ElementaryStepType, std::string> estype2str;

  /// @brief maps database label strings onto enum members
  static const std::map<std::string, StructureLabel> str2label;
  /// @brief maps label enum members onto database strings
  static const std::map<StructureLabel, std::string> label2str;
};

} /* namespace Layout */
} /* namespace Database */
} /* namespace Scine */

#endif
