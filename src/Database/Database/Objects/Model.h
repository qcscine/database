/**
 * @file Model.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_MODEL_H_
#define DATABASE_MODEL_H_

/* External Include */
#include <Utils/UniversalSettings/SettingsNames.h>
#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace bsoncxx {
inline namespace v_noabi {
namespace document {
class value;
class view;
} // namespace document
} // namespace v_noabi
} // namespace bsoncxx

namespace Scine {
namespace Utils {
class Settings;
} /* namespace Utils */
namespace Database {

/**
 * @class Model Model.h
 * @brief A data class containing information about the model used in a quantum chemical calculation.
 *
 * The main information keys are:
 *  - the method_family: i.e. DFT, CC, HF, AM1
 *  - the method: i.e. PBE0, CCSD, HF, AM1
 *  - the basis set: i.e. def2-tzvp, or also an empty string for semi-empirical methods
 *  - the spin mode: restricted, unrestricted
 *
 * Note that spin/multiplicity and charge are not part of this construct but are part of the
 * definition of a structure. See Structure.h.
 */
class Model {
 public:
  /**
   * @brief Construct a new Model.
   * @param mf The model family (DFT, CC, etc.).
   * @param m  The actual model (PBE0, CCSD etc.).
   * @param bs The basis set label (def2-tzvp, cc-pvtz, etc.).
   * @param sm The spin mode for the calculation (restricted, unrestricted).
   */
  Model(std::string mf, std::string m, std::string bs, std::string sm);
  /**
   * @brief Construct a new Model.
   * @param mf The model family (DFT, CC, etc.).
   * @param m  The actual model (PBE0, CCSD etc.).
   * @param bs The basis set label (def2-tzvp, cc-pvtz, etc.).
   */
  Model(std::string mf, std::string m, std::string bs);
  /**
   * @brief Construct a new Model.
   * @param bson The binary JSON document containing the model information.
   */
  Model(bsoncxx::v_noabi::document::view bson);
  /**
   * @brief Uses a set of Utils::Settings to complete fields in the Model
   *        that are set to any. The other fields will be checked for matching
   *        content if they exist.
   * @param settings The settings to be used for the completion.
   */
  void completeModel(const Utils::Settings& settings);
  /**
   * @brief Sets the settings of the Model in the given Settings object.
   * @param settings The settings to be used for the completion.
   */
  void completeSettings(Utils::Settings& settings);
  /**
   * @brief Gives a simple string representation of all model fields.
   * @return std::string A string with each field in one separate line.
   */
  std::string getStringRepresentation() const;

  std::string spinMode = "any";
  std::string basisSet;
  std::string method;
  std::string methodFamily;

  std::string program = "any";
  std::string version = "any";

  std::string temperature = "298.15";
  std::string electronicTemperature = "any";

  std::string solvation = "none";
  std::string solvent = "none";

  std::string embedding = "none";
  std::string periodicBoundaries = "none";
  std::string externalField = "none";

  void fromBson(bsoncxx::v_noabi::document::view bson);
  bsoncxx::v_noabi::document::value toBson() const;
  /**
   * @brief Compares two models for equality
   *
   * @note lhs.field: 'any' and rhs.field: 'something' are considered equal
   *       lhs.field: 'none' and rhs.field: '' are considered equal
   *       lhs.field: 'any' and rhs.field: 'none' or '' are NOT considered equal
   */
  bool operator==(const Model& rhs) const;
  bool operator!=(const Model& rhs) const;

  /**
   * @brief If the given entry is interpreted as None, i.e., and empty string or case insensitive 'none'
   * @param entry The entry in question
   * @return bool whether it is None or not
   */
  static inline bool entryIsNone(std::string entry) {
    std::for_each(entry.begin(), entry.end(), [](char& c) { c = ::tolower(c); });
    return entry.empty() || entry == "none";
  };
  /**
   * @brief If the given entry is case insensitive "any"
   * @param entry The entry in question
   * @return bool whether it is "any" or not
   */
  static inline bool entryIsAny(std::string entry) {
    std::for_each(entry.begin(), entry.end(), [](char& c) { c = ::tolower(c); });
    return entry == "any";
  };

 private:
  static const std::vector<std::string>& skipFields();

  std::map<std::string, std::reference_wrapper<std::string>> getSettingsModelPairs() {
    // reference wrapper instead of unique ptr because of std::pair
    std::map<std::string, std::reference_wrapper<std::string>> settingsModelPairs;
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::spinMode, std::ref(spinMode)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::basisSet, std::ref(basisSet)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::method, std::ref(method)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::methodFamily, std::ref(methodFamily)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::program, std::ref(program)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::version, std::ref(version)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::temperature, std::ref(temperature)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::electronicTemperature, std::ref(electronicTemperature)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::solvation, std::ref(solvation)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::solvent, std::ref(solvent)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::embedding, std::ref(embedding)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::periodicBoundaries, std::ref(periodicBoundaries)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::externalField, std::ref(externalField)));
    return settingsModelPairs;
  };

  std::map<std::string, std::reference_wrapper<const std::string>> getConstSettingsModelPairs() const {
    // reference wrapper instead of unique ptr because of std::pair
    std::map<std::string, std::reference_wrapper<const std::string>> settingsModelPairs;
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::spinMode, std::cref(spinMode)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::basisSet, std::cref(basisSet)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::method, std::cref(method)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::methodFamily, std::cref(methodFamily)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::program, std::cref(program)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::version, std::cref(version)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::temperature, std::cref(temperature)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::electronicTemperature, std::cref(electronicTemperature)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::solvation, std::cref(solvation)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::solvent, std::cref(solvent)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::embedding, std::cref(embedding)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::periodicBoundaries, std::cref(periodicBoundaries)));
    settingsModelPairs.emplace(std::make_pair(Utils::SettingsNames::externalField, std::cref(externalField)));
    return settingsModelPairs;
  };
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_MODEL_H_ */
