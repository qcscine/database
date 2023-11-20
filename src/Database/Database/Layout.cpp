/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include "Database/Layout.h"
#include "Database/Objects/Calculation.h"
#include "Database/Objects/ElementaryStep.h"

namespace Scine {
namespace Database {
namespace Layout {

constexpr const char* DefaultCollection::structure;
constexpr const char* DefaultCollection::calculation;
constexpr const char* DefaultCollection::elementaryStep;
constexpr const char* DefaultCollection::property;
constexpr const char* DefaultCollection::reaction;
constexpr const char* DefaultCollection::compound;
constexpr decltype(DefaultCollection::all) DefaultCollection::all;

// clang-format off
const std::map<std::string, CalculationStatus> EnumMaps::str2status = {
  {"construction", CalculationStatus::CONSTRUCTION},
  {"new", CalculationStatus::NEW},
  {"pending", CalculationStatus::PENDING},
  {"complete", CalculationStatus::COMPLETE},
  {"analyzed", CalculationStatus::ANALYZED},
  {"hold", CalculationStatus::HOLD},
  {"failed", CalculationStatus::FAILED}
};
const std::map<CalculationStatus, std::string> EnumMaps::status2str = {
  {CalculationStatus::CONSTRUCTION, "construction"},
  {CalculationStatus::NEW, "new"},
  {CalculationStatus::PENDING, "pending"},
  {CalculationStatus::COMPLETE, "complete"},
  {CalculationStatus::ANALYZED, "analyzed"},
  {CalculationStatus::HOLD, "hold"},
  {CalculationStatus::FAILED, "failed"}
};
// clang-format on

// clang-format off
const std::map<std::string, ElementaryStepType> EnumMaps::str2estype = {
  {"regular", ElementaryStepType::REGULAR},
  {"barrierless", ElementaryStepType::BARRIERLESS},
  {"model_transformation", ElementaryStepType::MODEL_TRANSFORMATION}
};
const std::map<ElementaryStepType, std::string> EnumMaps::estype2str = {
  {ElementaryStepType::REGULAR, "regular"},
  {ElementaryStepType::BARRIERLESS, "barrierless"},
  {ElementaryStepType::MODEL_TRANSFORMATION, "model_transformation"}
};
// clang-format on

// clang-format off
const std::map<std::string, StructureLabel> EnumMaps::str2label = {
  {"none", StructureLabel::NONE},
  {"user_guess", StructureLabel::USER_GUESS},
  {"user_optimized", StructureLabel::USER_OPTIMIZED},
  {"minimum_guess", StructureLabel::MINIMUM_GUESS},
  {"minimum_optimized", StructureLabel::MINIMUM_OPTIMIZED},
  {"ts_guess", StructureLabel::TS_GUESS},
  {"ts_optimized", StructureLabel::TS_OPTIMIZED},
  {"elementary_step_guess", StructureLabel::ELEMENTARY_STEP_GUESS},
  {"elementary_step_optimized", StructureLabel::ELEMENTARY_STEP_OPTIMIZED},
  {"reactive_complex_guess", StructureLabel::REACTIVE_COMPLEX_GUESS},
  {"reactive_complex_scanned", StructureLabel::REACTIVE_COMPLEX_SCANNED},
  {"reactive_complex_optimized", StructureLabel::REACTIVE_COMPLEX_OPTIMIZED},
  {"surface_guess", StructureLabel::SURFACE_GUESS},
  {"surface_optimized", StructureLabel::SURFACE_OPTIMIZED},
  {"surface_adsorption_guess", StructureLabel::SURFACE_ADSORPTION_GUESS},
  {"surface_complex_optimized", StructureLabel::SURFACE_COMPLEX_OPTIMIZED},
  {"user_surface_optimized", StructureLabel::USER_SURFACE_OPTIMIZED},
  {"user_surface_complex_optimized", StructureLabel::USER_SURFACE_COMPLEX_OPTIMIZED},
  {"complex_guess", StructureLabel::COMPLEX_GUESS},
  {"complex_optimized", StructureLabel::COMPLEX_OPTIMIZED},
  {"user_complex_optimized", StructureLabel::USER_COMPLEX_OPTIMIZED},
  {"irrelevant", StructureLabel::IRRELEVANT},
  {"duplicate", StructureLabel::DUPLICATE},
  {"geometry_optimization_observer", StructureLabel::GEOMETRY_OPTIMIZATION_OBSERVER},
  {"ts_optimization_observer", StructureLabel::TS_OPTIMIZATION_OBSERVER},
  // Storing Intrinsic Reaction Coordinate (IRC) steps
  {"irc_forward_observer", StructureLabel::IRC_FORWARD_OBSERVER},
  {"irc_backward_observer", StructureLabel::IRC_BACKWARD_OBSERVER},
  {"irc_opt_forward_observer", StructureLabel::IRC_OPT_FORWARD_OBSERVER},
  {"irc_opt_backward_observer", StructureLabel::IRC_OPT_BACKWARD_OBSERVER},
  {"scan_observer", StructureLabel::SCAN_OBSERVER}
};

const std::map<StructureLabel, std::string> EnumMaps::label2str = {
  {StructureLabel::NONE , "none"},
  {StructureLabel::USER_GUESS , "user_guess"},
  {StructureLabel::USER_OPTIMIZED , "user_optimized"},
  {StructureLabel::MINIMUM_GUESS , "minimum_guess"},
  {StructureLabel::MINIMUM_OPTIMIZED , "minimum_optimized"},
  {StructureLabel::TS_GUESS , "ts_guess"},
  {StructureLabel::TS_OPTIMIZED , "ts_optimized"},
  {StructureLabel::ELEMENTARY_STEP_GUESS , "elementary_step_guess"},
  {StructureLabel::ELEMENTARY_STEP_OPTIMIZED , "elementary_step_optimized"},
  {StructureLabel::REACTIVE_COMPLEX_GUESS , "reactive_complex_guess"},
  {StructureLabel::REACTIVE_COMPLEX_SCANNED , "reactive_complex_scanned"},
  {StructureLabel::REACTIVE_COMPLEX_OPTIMIZED , "reactive_complex_optimized"},
  {StructureLabel::SURFACE_GUESS , "surface_guess"},
  {StructureLabel::SURFACE_OPTIMIZED , "surface_optimized"},
  {StructureLabel::SURFACE_ADSORPTION_GUESS , "surface_adsorption_guess"},
  {StructureLabel::SURFACE_COMPLEX_OPTIMIZED , "surface_complex_optimized"},
  {StructureLabel::USER_SURFACE_OPTIMIZED , "user_surface_optimized"},
  {StructureLabel::USER_SURFACE_COMPLEX_OPTIMIZED, "user_surface_complex_optimized"},
  {StructureLabel::COMPLEX_GUESS, "complex_guess"},
  {StructureLabel::COMPLEX_OPTIMIZED, "complex_optimized"},
  {StructureLabel::USER_COMPLEX_OPTIMIZED, "user_complex_optimized"},
  {StructureLabel::IRRELEVANT, "irrelevant"},
  {StructureLabel::DUPLICATE, "duplicate"},
  {StructureLabel::GEOMETRY_OPTIMIZATION_OBSERVER, "geometry_optimization_observer"},
  {StructureLabel::TS_OPTIMIZATION_OBSERVER, "ts_optimization_observer"},
  {StructureLabel::IRC_FORWARD_OBSERVER, "irc_forward_observer"},
  {StructureLabel::IRC_BACKWARD_OBSERVER, "irc_backward_observer"},
  {StructureLabel::IRC_OPT_FORWARD_OBSERVER, "irc_opt_forward_observer"},
  {StructureLabel::IRC_OPT_BACKWARD_OBSERVER, "irc_opt_backward_observer"},
  {StructureLabel::SCAN_OBSERVER, "scan_observer"}
};
// clang-format on

} /* namespace Layout */
} /* namespace Database */
} /* namespace Scine */
