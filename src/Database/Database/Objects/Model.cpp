/**
 * @file Model.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Include */
#include "Database/Objects/Model.h"
/* External Includes */
#include <Utils/DataStructures/PeriodicBoundaries.h>
#include <Utils/Settings.h>
#include <Utils/Strings.h>
#include <math.h>
#include <bsoncxx/builder/stream/document.hpp>
#include <string>

namespace Scine {
namespace Database {

using namespace Scine::Utils;

Model::Model(std::string mf, std::string m, std::string bs, std::string sm)
  : spinMode(std::move(sm)), basisSet(std::move(bs)), method(std::move(m)), methodFamily(std::move(mf)) {
}

Model::Model(std::string mf, std::string m, std::string bs)
  : spinMode("any"), basisSet(std::move(bs)), method(std::move(m)), methodFamily(std::move(mf)) {
}

Model::Model(bsoncxx::document::view bson) {
  this->fromBson(bson);
}

void Model::fromBson(bsoncxx::document::view bson) {
  auto settingsModelPairs = getSettingsModelPairs(); // necessary to call here because of pybind
  for (auto& settingModelPair : settingsModelPairs) {
    settingModelPair.second.get() = bson[settingModelPair.first].get_utf8().value.to_string();
  }
}

bsoncxx::document::value Model::toBson() const {
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::finalize;
  auto bson = document{};
  const auto settingsModelPairs = getConstSettingsModelPairs(); // necessary to call here because of pybind
  for (const auto& settingModelPair : settingsModelPairs) {
    bson << std::string(settingModelPair.first) << settingModelPair.second.get();
  }
  return bson << finalize;
}

void Model::completeSettings(Utils::Settings& settings) {
  const auto settingsModelPairs = getSettingsModelPairs(); // necessary to call here because of pybind
  const auto& _skipFields = skipFields();

  /*
   * all but special fields
   */
  for (const auto& settingModelPair : settingsModelPairs) {
    const auto settingName = settingModelPair.first;
    const auto modelEntryRef = settingModelPair.second;
    /*
     * method_family, program, and version
     * need to be set with knowledge of the calculator, they can not be set using just Settings.
     * temperature, pressure, and electronicTemperature are set differently below
     */
    if (std::find(_skipFields.begin(), _skipFields.end(), settingName) != _skipFields.end()) {
      continue;
    }
    const bool exists = settings.valueExists(settingName);
    if (!exists && !entryIsNone(modelEntryRef.get())) {
      throw std::runtime_error("Setting " + settingName + " does not exist in settings, but has the value " +
                               modelEntryRef.get() + " in the model.");
    }
    if (exists) {
      // if model field is 'any' and settings have a value that is NOT 'none', we keep the settings default
      if (entryIsAny(modelEntryRef.get()) && !entryIsNone(settings.getString(settingName))) {
        continue;
      }
      settings.modifyString(settingName, modelEntryRef.get());
    }
  }
  /*
   * temperatures / pressure
   */
  std::vector<std::string> temperatureSettings = {SettingsNames::temperature, SettingsNames::electronicTemperature,
                                                  SettingsNames::pressure};
  for (const auto& tempSettingName : temperatureSettings) {
    const bool exists = settings.valueExists(tempSettingName);
    auto modelEntryRef = settingsModelPairs.at(tempSettingName);
    if (!exists) {
      if (!entryIsNone(modelEntryRef.get())) {
        throw std::runtime_error("Setting '" + tempSettingName + "' does not exist in settings, but has the value '" +
                                 modelEntryRef.get() + "' in the model.");
      }
      // setting does not exists and model entry is none --> no need for manipulation
      continue;
    }
    // we do not allow 'none' for temperatures if the setting exists
    if (exists && entryIsNone(modelEntryRef.get())) {
      throw std::runtime_error("Settings expect a value for " + tempSettingName + ", not 'none'.");
    }
    // we do not allow 'any' for temperatures, model entry is ignored and default of settings is kept
    if (!entryIsAny(modelEntryRef.get())) {
      settings.modifyDouble(tempSettingName, std::stod(modelEntryRef.get()));
    }
  }
}

void Model::completeModel(const Utils::Settings& settings) {
  const auto& _skipFields = skipFields();
  const auto settingsModelPairs = getSettingsModelPairs(); // necessary to call here because of pybind
  /*
   * all but special fields
   */
  for (const auto& settingModelPair : settingsModelPairs) {
    const auto settingName = settingModelPair.first;
    const auto modelEntryRef = settingModelPair.second;
    /*
     * method_family, program, and version
     * need to be set with knowledge of the calculator, they can not be set using just Settings.
     * temperature, pressure, and electronicTemperature are set differently below
     */
    if (std::find(_skipFields.begin(), _skipFields.end(), settingName) != _skipFields.end()) {
      continue;
    }
    const bool exists = settings.valueExists(settingName);
    if (exists && !entryIsAny(modelEntryRef.get())) {
      if (!Utils::caseInsensitiveEqual(settings.getString(settingName), modelEntryRef.get())) {
        // setting and model mismatch literally, make sure they are not different versions of 'none'
        if (entryIsNone(settings.getString(settingName)) && entryIsNone(modelEntryRef.get())) {
          continue;
        }
        throw std::runtime_error("The setting '" + settingName + "' has the value '" + settings.getString(settingName) +
                                 "', which is different to the value of '" + modelEntryRef.get() +
                                 "' in the model. The model field can only be changed if it is 'any'.");
      }
    }
    else if (exists) {
      modelEntryRef.get() = settings.getString(settingName);
    }
    else if (entryIsNone(modelEntryRef.get())) {
      // setting does not exist, but model also requires none -> nothing to do and no error
    }
    else {
      throw std::runtime_error("Settings are missing the key '" + settingName +
                               "', which is not 'none' in the model, but has the value '" + modelEntryRef.get() + "'.");
    }
  }
  /*
   * temperatures / pressure
   */
  std::vector<std::string> temperatureSettings = {SettingsNames::temperature, SettingsNames::electronicTemperature,
                                                  SettingsNames::pressure};
  for (const auto& tempSettingName : temperatureSettings) {
    const bool exists = settings.valueExists(tempSettingName);
    auto modelEntryRef = settingsModelPairs.at(tempSettingName);
    if (entryIsAny(modelEntryRef.get())) {
      modelEntryRef.get() = (exists) ? std::to_string(settings.getDouble(tempSettingName)) : "none";
    }
    else if (!exists && entryIsNone(modelEntryRef.get())) {
      // setting does not exist, but model also requires none -> nothing to do and no error
    }
    else if (!exists) {
      throw std::runtime_error("Settings are missing the key '" + tempSettingName +
                               "', which is not 'none' in the model, but has the value '" + modelEntryRef.get() + "'.");
    }
    // we now know that model entry is not none and not any
    else if (std::fabs(std::stod(modelEntryRef.get()) - settings.getDouble(tempSettingName)) > 1e-12) {
      throw std::runtime_error("Settings have the value of '" + std::to_string(settings.getDouble(tempSettingName)) +
                               "' for the setting '" + tempSettingName + "' and the model has the value of '" +
                               modelEntryRef.get() + "'. Different values cannot be harmonized.");
    }
    // else model and settings match temperature
  }
}

std::string Model::getStringRepresentation() const {
  std::string output = "Model:\n";
  const auto map = getConstSettingsModelPairs();
  for (const auto& pair : map) {
    std::string line = pair.first + " : " + pair.second.get() + "\n";
    output += line;
  }
  return output;
}

const std::vector<std::string>& Model::skipFields() {
  static std::vector<std::string> fields = {Utils::SettingsNames::methodFamily,
                                            Utils::SettingsNames::program,
                                            Utils::SettingsNames::version,
                                            Utils::SettingsNames::temperature,
                                            Utils::SettingsNames::electronicTemperature,
                                            Utils::SettingsNames::pressure};
  return fields;
}

bool Model::operator==(const Model& rhs) const {
  return equality(rhs, true);
}

bool Model::equalWithoutPeriodicBoundaryCheck(const Model& rhs) const {
  return equality(rhs, false);
}

bool Model::equality(const Model& rhs, bool comparePbc) const {
  auto lhsPairs = this->getConstSettingsModelPairs(); // necessary to call here because of pybind
  auto rhsPairs = rhs.getConstSettingsModelPairs();
  auto rhsIterator = rhsPairs.begin();
  std::array<std::string, 3> doubleSettings = {SettingsNames::temperature, SettingsNames::electronicTemperature,
                                               SettingsNames::pressure};
  for (auto& settingModelPair : lhsPairs) {
    auto lhsModelEntryRef = settingModelPair.second;
    auto rhsModelEntryRef = rhsIterator->second;
    if ((entryIsAny(lhsModelEntryRef.get()) && !entryIsNone(rhsModelEntryRef.get())) ||
        (entryIsAny(rhsModelEntryRef.get()) && !entryIsNone(lhsModelEntryRef.get())) ||
        (entryIsNone(lhsModelEntryRef.get()) && entryIsNone(rhsModelEntryRef.get()))) {
      rhsIterator++;
      continue;
    }
    if (!Utils::caseInsensitiveEqual(lhsModelEntryRef.get(), rhsModelEntryRef.get())) {
      // make sure we don't have a potential double value
      std::string name = settingModelPair.first;
      if (std::find(doubleSettings.begin(), doubleSettings.end(), name) != doubleSettings.end()) {
        // double values are possible
        // we are doing try/catch instead of C style checks because the catch case should be uncommon and try has zero
        // cost
        try {
          auto lhsDouble = std::stod(lhsModelEntryRef.get());
          auto rhsDouble = std::stod(rhsModelEntryRef.get());
          if ((std::fabs(lhsDouble) - std::fabs(rhsDouble)) < 1e-12) {
            // double equality
            rhsIterator++;
            continue;
          }
        }
        catch (...) {
          // could not cast to double; any/none have been handled earlier
          return false;
        }
      }
      // make sure we don't have equivalent periodic boundaries
      else if (name == Utils::SettingsNames::periodicBoundaries) {
        if (!comparePbc) {
          rhsIterator++;
          continue;
        }
        try {
          auto lhsPbc = Utils::PeriodicBoundaries(lhsModelEntryRef.get());
          auto rhsPbc = Utils::PeriodicBoundaries(rhsModelEntryRef.get());
          if (lhsPbc == rhsPbc) {
            // pbc equality
            rhsIterator++;
            continue;
          }
        }
        catch (...) {
          // were not valid PBC, possible for 'any' vs 'none' case where we want a 'false' anyway
          return false;
        }
      }
      return false;
    }
    rhsIterator++;
  }
  return true;
}

bool Model::operator!=(const Model& rhs) const {
  return !(*this == rhs);
}

} /* namespace Database */
} /* namespace Scine */
