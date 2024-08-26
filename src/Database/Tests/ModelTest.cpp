/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include "Database/Objects/Model.h"
#include <Utils/Settings.h>
#include <Utils/UniversalSettings/SettingsNames.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {

using namespace Scine::Utils;

class ModelSettings : public Scine::Utils::Settings {
 public:
  ModelSettings() : Scine::Utils::Settings("ModelSettings") {
    using namespace Scine::Utils::UniversalSettings;

    // temperature
    DoubleDescriptor temperature("");
    temperature.setDefaultValue(298.15);
    this->_fields.push_back(SettingsNames::temperature, temperature);
    // temperature
    DoubleDescriptor pressure("");
    pressure.setDefaultValue(1e+5);
    this->_fields.push_back(SettingsNames::pressure, pressure);
    // electronicTemperature
    DoubleDescriptor electronicTemperature("");
    electronicTemperature.setDefaultValue(300.0);
    this->_fields.push_back(SettingsNames::electronicTemperature, electronicTemperature);
    // method
    StringDescriptor method("");
    method.setDefaultValue("");
    this->_fields.push_back(SettingsNames::method, method);
    // methodFamily
    StringDescriptor methodFamily("");
    methodFamily.setDefaultValue("");
    this->_fields.push_back(SettingsNames::methodFamily, methodFamily);
    // spinMode
    StringDescriptor spinMode("");
    spinMode.setDefaultValue("");
    this->_fields.push_back(SettingsNames::spinMode, spinMode);
    // program
    StringDescriptor program("");
    program.setDefaultValue("");
    this->_fields.push_back(SettingsNames::program, program);
    // version
    StringDescriptor version("");
    version.setDefaultValue("");
    this->_fields.push_back(SettingsNames::version, version);
    // basisSet
    StringDescriptor basisSet("");
    basisSet.setDefaultValue("");
    this->_fields.push_back(SettingsNames::basisSet, basisSet);
    // solvation
    StringDescriptor solvation("");
    solvation.setDefaultValue("");
    this->_fields.push_back(SettingsNames::solvation, solvation);
    // Solvent
    StringDescriptor solvent("");
    solvent.setDefaultValue("");
    this->_fields.push_back(SettingsNames::solvent, solvent);
    // embedding
    StringDescriptor embedding("");
    embedding.setDefaultValue("");
    this->_fields.push_back(SettingsNames::embedding, embedding);
    // periodicBoundaries
    StringDescriptor periodicBoundaries("");
    periodicBoundaries.setDefaultValue("");
    this->_fields.push_back(SettingsNames::periodicBoundaries, periodicBoundaries);
    // externalField
    StringDescriptor externalField("");
    externalField.setDefaultValue("");
    this->_fields.push_back(SettingsNames::externalField, externalField);

    this->resetToDefaults();
  }
};

/**
 * @class ModelTest ModelTest.cpp
 * @brief Comprises tests for the class Scine::Database::Model.
 * @test
 */
class ModelTest : public Test {};

TEST_F(ModelTest, CompleteModel) {
  ModelSettings settings;
  settings.modifyDouble(SettingsNames::temperature, 1.0);
  settings.modifyDouble(SettingsNames::electronicTemperature, 2.0);
  settings.modifyDouble(SettingsNames::pressure, 1e+5);
  settings.modifyString(SettingsNames::method, "3");
  settings.modifyString(SettingsNames::methodFamily, "4");
  settings.modifyString(SettingsNames::spinMode, "5");
  settings.modifyString(SettingsNames::program, "6");
  settings.modifyString(SettingsNames::version, "7");
  settings.modifyString(SettingsNames::basisSet, "8");
  settings.modifyString(SettingsNames::solvation, "9");
  settings.modifyString(SettingsNames::solvent, "10");
  settings.modifyString(SettingsNames::embedding, "11");
  settings.modifyString(SettingsNames::periodicBoundaries, "12");
  settings.modifyString(SettingsNames::externalField, "none");

  Model model("should-not-change", "any", "any");
  model.program = "should-not-change";
  model.version = "should-not-change";
  model.spinMode = "ANY";
  model.temperature = "any";
  model.electronicTemperature = "any";
  model.solvation = "any";
  model.solvent = "any";
  model.embedding = "any";
  model.periodicBoundaries = "any";
  model.externalField = "";
  model.pressure = "1e+5";

  model.completeModel(settings);

  EXPECT_EQ(model.temperature, std::to_string(1.0));
  EXPECT_EQ(model.electronicTemperature, std::to_string(2.0));
  EXPECT_EQ(model.method, "3");
  EXPECT_EQ(model.methodFamily, "should-not-change");
  EXPECT_EQ(model.spinMode, "5");
  EXPECT_EQ(model.version, "should-not-change");
  EXPECT_EQ(model.program, "should-not-change");
  EXPECT_EQ(model.basisSet, "8");
  EXPECT_EQ(model.solvation, "9");
  EXPECT_EQ(model.solvent, "10");
  EXPECT_EQ(model.embedding, "11");
  EXPECT_EQ(model.periodicBoundaries, "12");
  EXPECT_EQ(model.externalField, "");
  EXPECT_EQ(model.pressure, "1e+5");

  // check if we throw because of collision
  model.embedding = "something";
  EXPECT_THROW(model.completeModel(settings), std::runtime_error);
}

TEST_F(ModelTest, CompleteSettings) {
  ModelSettings settings;
  settings.modifyString(SettingsNames::methodFamily, "should-not-change");
  settings.modifyString(SettingsNames::program, "should-not-change");
  settings.modifyString(SettingsNames::version, "should-not-change");
  settings.modifyString(SettingsNames::spinMode, "ANY");
  settings.modifyString(SettingsNames::embedding, "any");

  Model model("should-not-change", "any", "def2-tzvp");
  model.program = "should-not-change";
  model.version = "should-not-change";
  model.spinMode = "unrestricted";
  model.temperature = "any";
  model.electronicTemperature = "0.0";
  model.solvation = "pcm";
  model.solvent = "water";
  model.embedding = "fde";
  model.periodicBoundaries = "none";
  model.externalField = "";

  model.completeSettings(settings);

  EXPECT_EQ(settings.getDouble(SettingsNames::temperature), 298.15);
  EXPECT_EQ(settings.getDouble(SettingsNames::electronicTemperature), 0.0);
  EXPECT_EQ(settings.getString(SettingsNames::method), "any");
  EXPECT_EQ(settings.getString(SettingsNames::methodFamily), "should-not-change");
  EXPECT_EQ(settings.getString(SettingsNames::spinMode), "unrestricted");
  EXPECT_EQ(settings.getString(SettingsNames::version), "should-not-change");
  EXPECT_EQ(settings.getString(SettingsNames::program), "should-not-change");
  EXPECT_EQ(settings.getString(SettingsNames::basisSet), "def2-tzvp");
  EXPECT_EQ(settings.getString(SettingsNames::solvation), "pcm");
  EXPECT_EQ(settings.getString(SettingsNames::solvent), "water");
  EXPECT_EQ(settings.getString(SettingsNames::embedding), "fde");
  EXPECT_EQ(settings.getString(SettingsNames::periodicBoundaries), "none");
  EXPECT_EQ(settings.getString(SettingsNames::externalField), "");

  // check if we throw for 'none' temperature
  model.temperature = "none";
  EXPECT_THROW(model.completeSettings(settings), std::runtime_error);
}

TEST_F(ModelTest, EqualityWorks) {
  Model lhs("dft", "any", "none", "none");
  Model rhs("dft", "something", "", "none");
  ASSERT_TRUE(lhs == rhs);
  ASSERT_TRUE(lhs.equalWithoutPeriodicBoundaryCheck(rhs));
  lhs.program = "sparrow";
  rhs.program = "Sparrow";
  ASSERT_TRUE(lhs == rhs);
  ASSERT_TRUE(lhs.equalWithoutPeriodicBoundaryCheck(rhs));
  rhs.program = "something_different";
  ASSERT_FALSE(lhs == rhs);
  ASSERT_FALSE(lhs.equalWithoutPeriodicBoundaryCheck(rhs));
  lhs.program = "any";
  rhs.program = "something";
  ASSERT_TRUE(lhs == rhs);
  rhs.program = "none";
  ASSERT_FALSE(lhs == rhs);
  rhs.program = "";
  ASSERT_FALSE(lhs == rhs);
  rhs.program = "sparrow";
  ASSERT_TRUE(lhs == rhs);
  lhs.temperature = "373.15";
  rhs.temperature = "any";
  ASSERT_TRUE(lhs == rhs);
  rhs.temperature = "373.150";
  ASSERT_TRUE(lhs == rhs);
  rhs.temperature = "none";
  ASSERT_FALSE(lhs == rhs);
  rhs.temperature = "Any";
  ASSERT_TRUE(lhs == rhs);
  lhs.periodicBoundaries = "any";
  rhs.periodicBoundaries = "ANY";
  ASSERT_TRUE(lhs == rhs);
  ASSERT_TRUE(lhs.equalWithoutPeriodicBoundaryCheck(rhs));
  rhs.periodicBoundaries = "none";
  ASSERT_TRUE(lhs.equalWithoutPeriodicBoundaryCheck(rhs));
  ASSERT_FALSE(lhs == rhs);
  rhs.periodicBoundaries = "8.0,10.0,10.0,45.0,90.0,90.0,xyz";
  ASSERT_TRUE(lhs == rhs);
  lhs.periodicBoundaries = "8.0,10.0,10.0,45.0,90.0,90.0,xyz";
  ASSERT_TRUE(lhs == rhs);
  rhs.periodicBoundaries = "8.00,10.0,10.0,45.0,90.0,90.00,xyz";
  ASSERT_TRUE(lhs == rhs);
  ASSERT_TRUE(lhs.equalWithoutPeriodicBoundaryCheck(rhs));
  rhs.periodicBoundaries = "8.00,10.1,10.0,45.0,90.0,90.00,xyz";
  ASSERT_FALSE(lhs == rhs);
  ASSERT_TRUE(lhs.equalWithoutPeriodicBoundaryCheck(rhs));
}

TEST_F(ModelTest, EntryNoneAndAnyChecksWork) {
  std::string test = "any";
  ASSERT_TRUE(Model::entryIsAny(test));
  ASSERT_FALSE(Model::entryIsNone(test));
  test = "ANY";
  ASSERT_TRUE(Model::entryIsAny(test));
  ASSERT_FALSE(Model::entryIsNone(test));
  test = "aNy";
  ASSERT_TRUE(Model::entryIsAny(test));
  ASSERT_FALSE(Model::entryIsNone(test));

  test = "none";
  ASSERT_FALSE(Model::entryIsAny(test));
  ASSERT_TRUE(Model::entryIsNone(test));
  test = "NONE";
  ASSERT_FALSE(Model::entryIsAny(test));
  ASSERT_TRUE(Model::entryIsNone(test));
  test = "nOne";
  ASSERT_FALSE(Model::entryIsAny(test));
  ASSERT_TRUE(Model::entryIsNone(test));
  test = "";
  ASSERT_FALSE(Model::entryIsAny(test));
  ASSERT_TRUE(Model::entryIsNone(test));

  test = "something";
  ASSERT_FALSE(Model::entryIsAny(test));
  ASSERT_FALSE(Model::entryIsNone(test));
}

TEST_F(ModelTest, OutputStringWorks) {
  Model m("any", "any", "any", "any");
  std::vector<std::string> names = {SettingsNames::temperature,   SettingsNames::electronicTemperature,
                                    SettingsNames::method,        SettingsNames::methodFamily,
                                    SettingsNames::spinMode,      SettingsNames::program,
                                    SettingsNames::version,       SettingsNames::basisSet,
                                    SettingsNames::solvation,     SettingsNames::solvent,
                                    SettingsNames::embedding,     SettingsNames::periodicBoundaries,
                                    SettingsNames::externalField, SettingsNames::pressure};
  m.temperature = "0";
  m.electronicTemperature = "1";
  m.method = "2";
  m.methodFamily = "3";
  m.spinMode = "4";
  m.program = "5";
  m.version = "6";
  m.basisSet = "7";
  m.solvation = "8";
  m.solvent = "9";
  m.embedding = "10";
  m.periodicBoundaries = "11";
  m.externalField = "12";
  std::stringstream s(m.getStringRepresentation());
  std::string line;
  std::getline(s, line); // skip first
  while (std::getline(s, line)) {
    bool found = false;
    for (unsigned long i = 0; i < names.size(); ++i) {
      std::string target = names[i] + std::string(" :");
      if (line.find(target) != std::string::npos) {
        found = line.find(std::to_string(i)) != std::string::npos;
      }
    }
    ASSERT_TRUE(found);
  }
}

} // namespace Tests
} // namespace Database
} // namespace Scine
