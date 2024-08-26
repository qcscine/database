/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Calculation.h>
#include <Database/Objects/Model.h>
// #include <Database/Objects/Compound.h>
// #include <Database/Objects/Structure.h>
#include <Utils/Geometry/AtomCollection.h>
#include <Utils/UniversalSettings/ParametrizedOptionValue.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class CalculationTest CalculationTest.cpp
 * @brief Comprises tests for the class Scine::Database::Calculation.
 * @test
 */
class CalculationTest : public Test {
 public:
  CalculationTest() {
  }
  static Credentials credentials;
  static Manager db;

  static void SetUpTestCase() {
    db.setCredentials(credentials);
    db.connect();
    db.init();
  }
  static void TearDownTestCase() {
    db.wipe();
  }
};
Credentials CalculationTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_CalculationTest");
Manager CalculationTest::db;

TEST_F(CalculationTest, Creation) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation::Job j("geo_opt");
  Model m("dft", "pbe", "def2-svp");
  Calculation calc = Calculation::create(m, j, {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Check Fields
  auto status = calc.getStatus();
  auto auxiliaries = calc.getAuxiliaries();
  auto restartInfo = calc.getRestartInformation();
  auto structures = calc.getStructures();
  auto settings = calc.getSettings();
  auto model = calc.getModel();
  auto results = calc.getResults();
  auto job = calc.getJob();
  auto output = calc.getRawOutput();
  auto comment = calc.getComment();

  ASSERT_EQ(status, Calculation::STATUS::CONSTRUCTION);
  ASSERT_EQ(auxiliaries.size(), 0);
  ASSERT_EQ(restartInfo.size(), 0);
  ASSERT_EQ(structures.size(), 3);
  ASSERT_EQ(settings.size(), 0);
  ASSERT_EQ(results.structures.size(), 0);
  ASSERT_EQ(results.properties.size(), 0);
  ASSERT_EQ(results.elementarySteps.size(), 0);
  ASSERT_EQ(model.method, "pbe");
  ASSERT_EQ(model.basisSet, "def2-svp");
  ASSERT_EQ(model.spinMode, "any");
  ASSERT_EQ(job.order, "geo_opt");
  ASSERT_EQ(output, "");
  ASSERT_EQ(comment, "");
}

TEST_F(CalculationTest, Priority) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation::Job job("geo_opt");
  Model model("dft", "pbe", "def2-svp");
  Calculation calc = Calculation::create(model, job, {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Priority Functionalities
  auto priority = calc.getPriority();
  ASSERT_EQ(priority, 10);
  ASSERT_NE(priority, 1);
  calc.setPriority(1);
  priority = calc.getPriority();
  ASSERT_EQ(priority, 1);
}

TEST_F(CalculationTest, PriorityFailsCollection) {
  Calculation calc;
  ASSERT_THROW(calc.getPriority(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setPriority(5), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, PriorityFailsID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ASSERT_THROW(calc.getPriority(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setPriority(1), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, PriorityFailsRange) {
  auto coll = db.getCollection("calculations");
  ID id;
  Calculation calc(id);
  calc.link(coll);
  ASSERT_THROW(calc.setPriority(0), std::invalid_argument);
  ASSERT_THROW(calc.setPriority(123), std::invalid_argument);
}

TEST_F(CalculationTest, Status) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation::Job job("geo_opt");
  Model model("dft", "pbe", "def2-svp");
  Calculation calc = Calculation::create(model, job, {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Status Functionalities
  auto status = calc.getStatus();
  ASSERT_EQ(status, Calculation::STATUS::CONSTRUCTION);
  ASSERT_NE(status, Calculation::STATUS::NEW);
  calc.setStatus(Calculation::STATUS::FAILED);
  status = calc.getStatus();
  ASSERT_EQ(calc.getStatus(), Calculation::STATUS::FAILED);
}

TEST_F(CalculationTest, StatusFailsCollection) {
  Calculation calc;
  ASSERT_THROW(calc.getStatus(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setStatus(Calculation::STATUS::NEW), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, StatusFailsID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ASSERT_THROW(calc.getStatus(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setStatus(Calculation::STATUS::NEW), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, Job) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation calc = Calculation::create(Model("dft", "pbe", "def2-svp"), Calculation::Job("geo_opt"), {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Job Functionalities
  auto job = calc.getJob();
  ASSERT_EQ(job.order, "geo_opt");
  job.order = "asdf";
  job.memory = 123.456;
  job.cores = 15;
  job.disk = 654.123;
  calc.setJob(job);
  auto job_db = calc.getJob();
  ASSERT_EQ(job.order, job_db.order);
  ASSERT_EQ(job.memory, job_db.memory);
  ASSERT_EQ(job.cores, job_db.cores);
  ASSERT_EQ(job.disk, job_db.disk);
}

TEST_F(CalculationTest, JobFailsCollection) {
  Calculation calc;
  ASSERT_THROW(calc.getJob(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setJob(Calculation::Job("geo_opt")), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, JobFailsID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ASSERT_THROW(calc.getJob(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setJob(Calculation::Job("geo_opt")), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, Model) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation calc =
      Calculation::create(Model("dft", "pbe", "def2-svp", "restricted"), Calculation::Job("geo_opt"), {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Model Functionalities
  auto model = calc.getModel();
  ASSERT_EQ(model.method, "pbe");
  ASSERT_EQ(model.basisSet, "def2-svp");
  ASSERT_EQ(model.spinMode, "restricted");
  model.spinMode = "a";
  model.basisSet = "b";
  model.spinMode = "c";
  model.program = "d";
  model.version = "e";
  model.solvation = "f";
  model.solvent = "g";
  model.embedding = "h";
  model.periodicBoundaries = "i";
  model.externalField = "j";
  calc.setModel(model);
  auto model_db = calc.getModel();
  ASSERT_EQ(model.spinMode, model_db.spinMode);
  ASSERT_EQ(model.basisSet, model_db.basisSet);
  ASSERT_EQ(model.spinMode, model_db.spinMode);
  ASSERT_EQ(model.program, model_db.program);
  ASSERT_EQ(model.version, model_db.version);
  ASSERT_EQ(model.solvation, model_db.solvation);
  ASSERT_EQ(model.solvent, model_db.solvent);
  ASSERT_EQ(model.embedding, model_db.embedding);
  ASSERT_EQ(model.periodicBoundaries, model_db.periodicBoundaries);
  ASSERT_EQ(model.externalField, model_db.externalField);
}

TEST_F(CalculationTest, ModelFailsCollection) {
  Calculation calc;
  ASSERT_THROW(calc.getModel(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setModel(Model("dft", "pbe", "def2-svp")), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, ModelFailsID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ASSERT_THROW(calc.getModel(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setModel(Model("dft", "pbe", "def2-svp")), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, Structures) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation calc = Calculation::create(Model("dft", "pbe", "def2-svp"), Calculation::Job("geo_opt"), {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Structure Functionalities
  ID s4, s5, s6;

  auto structures = calc.getStructures();
  ASSERT_EQ(structures.size(), 3);
  ASSERT_EQ(structures[0], s1);
  ASSERT_EQ(structures[1], s2);
  ASSERT_EQ(structures[2], s3);

  calc.addStructure(s4);
  calc.removeStructure(s1);
  structures = calc.getStructures();
  ASSERT_EQ(structures.size(), 3);
  ASSERT_EQ(structures[0], s2);
  ASSERT_EQ(structures[1], s3);
  ASSERT_EQ(structures[2], s4);

  calc.setStructures(std::vector<ID>{s4, s5, s6});
  structures = calc.getStructures();
  ASSERT_EQ(structures.size(), 3);
  ASSERT_EQ(structures[0], s4);
  ASSERT_EQ(structures[1], s5);
  ASSERT_EQ(structures[2], s6);
  ASSERT_TRUE(calc.hasStructure(s4));

  calc.clearStructures();
  structures = calc.getStructures();
  ASSERT_EQ(structures.size(), 0);
  ASSERT_FALSE(calc.hasStructure(s4));
}

TEST_F(CalculationTest, StructuresFailsCollection) {
  Calculation calc;
  ID id;
  ASSERT_THROW(calc.getStructures(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.hasStructure(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.removeStructure(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.addStructure(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.clearStructures(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setStructures(std::vector<ID>{}), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, StructuresFailsID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ID id;
  ASSERT_THROW(calc.getStructures(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.hasStructure(id), Exceptions::MissingIDException);
  ASSERT_THROW(calc.removeStructure(id), Exceptions::MissingIDException);
  ASSERT_THROW(calc.addStructure(id), Exceptions::MissingIDException);
  ASSERT_THROW(calc.clearStructures(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setStructures(std::vector<ID>{}), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, Settings) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation calc =
      Calculation::create(Model("dft", "pbe", "def2-svp", "restricted"), Calculation::Job("geo_opt"), {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Setting Functionalities
  auto settings = calc.getSettings();
  ASSERT_EQ(settings.size(), 0);

  const std::vector<std::vector<int>> listOfLists = {{0, 1, 2}, {3, 4, 5}, {-1, -2, 9}};
  calc.setSetting("foo", Utils::UniversalSettings::GenericValue::fromString("bar"));
  calc.setSetting("spam", Utils::UniversalSettings::GenericValue::fromInt(4));
  calc.setSetting("spam_with_eggs", Utils::UniversalSettings::GenericValue::fromIntListList(listOfLists));
  auto settings_db = calc.getSettings();
  ASSERT_EQ(settings_db.getString("foo"), "bar");
  ASSERT_EQ(settings_db.getInt("spam"), 4);
  ASSERT_EQ(calc.getSetting("foo").toString(), "bar");
  ASSERT_EQ(calc.getSetting("spam").toInt(), 4);
  const std::vector<std::vector<int>> listOfListsFromSettings = calc.getSetting("spam_with_eggs").toIntListList();
  ASSERT_EQ(listOfListsFromSettings.size(), listOfLists.size());
  for (unsigned int i = 0; i < listOfListsFromSettings.size(); ++i) {
    const std::vector<int> sublist = listOfListsFromSettings[i];
    const std::vector<int> sublistRef = listOfLists[i];
    ASSERT_EQ(sublist.size(), sublistRef.size());
    for (unsigned int j = 0; j < sublist.size(); ++j) {
      ASSERT_EQ(sublist[j], sublistRef[j]);
    }
  }

  calc.removeSetting("foo");
  settings_db = calc.getSettings();
  ASSERT_EQ(settings_db.getInt("spam"), 4);
  ASSERT_EQ(calc.getSetting("spam"), 4);
  ASSERT_TRUE(calc.hasSetting("spam"));

  calc.clearSettings();
  ASSERT_EQ(settings.size(), 0);

  using Utils::UniversalSettings::GenericValue;
  using Utils::UniversalSettings::ParametrizedOptionValue;
  using Utils::UniversalSettings::ValueCollection;

  ValueCollection fruitBowl{};
  fruitBowl.addInt("pear", 2);
  fruitBowl.addInt("fig", 5);

  ValueCollection climbingPlants{};
  climbingPlants.addBool("annoying", true);
  climbingPlants.addBool("difficult_removal", true);
  climbingPlants.addBool("pretty", true);
  climbingPlants.addInt("species", 401293);
  ParametrizedOptionValue plants{"climbing", climbingPlants};

  ValueCollection roomDecoration;
  roomDecoration.addOptionWithSettings("plants", plants);

  ValueCollection cars{};
  cars.addStringList("classic", {{"triumph", "pontiac", "plymouth"}});

  settings = {};
  settings.addString("foo", "bar");
  settings.addInt("spam", 4);
  const auto appleList = std::vector<std::string>{{"Hello", "I", "am", "an", "apple"}};
  const auto bananaList = std::vector<int>{{1, 2, 3, 4}};
  const auto orangeList = std::vector<double>{{5.0, 6.0, 7.0, 8.0}};
  const auto cherryList = GenericValue::CollectionList{{fruitBowl, roomDecoration, cars}};
  settings.addStringList("apple", appleList);
  settings.addIntList("banana", bananaList);
  settings.addDoubleList("orange", orangeList);
  settings.addCollectionList("cherry", cherryList);
  calc.setSettings(settings);
  ASSERT_EQ(calc.getSetting("foo"), std::string{"bar"});
  ASSERT_EQ(calc.getSetting("spam"), 4);
  ASSERT_EQ(calc.getSetting("apple"), appleList);
  ASSERT_EQ(calc.getSetting("banana"), bananaList);
  ASSERT_EQ(calc.getSetting("orange"), orangeList);
  ASSERT_EQ(calc.getSetting("cherry"), cherryList);
}

TEST_F(CalculationTest, SettingsFailCollection) {
  Calculation calc;
  ASSERT_THROW(calc.getSettings(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.getSetting("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.hasSetting("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.removeSetting("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setSetting("foo", Utils::UniversalSettings::GenericValue::fromString("bar")),
               Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setSettings(Utils::UniversalSettings::ValueCollection()), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.clearSettings(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, SettingsFailID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ASSERT_THROW(calc.getSettings(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.getSetting("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.hasSetting("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.removeSetting("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setSetting("foo", Utils::UniversalSettings::GenericValue::fromString("bar")), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setSettings(Utils::UniversalSettings::ValueCollection()), Exceptions::MissingIDException);
  ASSERT_THROW(calc.clearSettings(), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, Results) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation calc =
      Calculation::create(Model("dft", "pbe", "def2-svp", "restricted"), Calculation::Job("geo_opt"), {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());
  Calculation calc2 =
      Calculation::create(Model("dft", "pbe", "def2-svp", "restricted"), Calculation::Job("sp"), {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Result Functionalities
  ID id1, id2, id3, id4, id5, id6, id7, id8, id9;

  auto results = calc.getResults();
  ASSERT_EQ(results.structures.size(), 0);
  ASSERT_EQ(results.properties.size(), 0);
  ASSERT_EQ(results.elementarySteps.size(), 0);
  auto results2 = calc2.getResults();
  ASSERT_EQ(results2.structures.size(), 0);
  ASSERT_EQ(results2.properties.size(), 0);
  ASSERT_EQ(results2.elementarySteps.size(), 0);

  results.structures.push_back(id1);
  results.structures.push_back(id2);
  results.properties.push_back(id3);
  results.properties.push_back(id4);
  results.elementarySteps.push_back(id5);
  results.elementarySteps.push_back(id6);
  calc.setResults(results);
  auto results_db = calc.getResults();
  ASSERT_EQ(results_db.structures.size(), 2);
  ASSERT_EQ(results_db.properties.size(), 2);
  ASSERT_EQ(results_db.elementarySteps.size(), 2);
  ASSERT_EQ(results_db.structures[0], id1);
  ASSERT_EQ(results_db.structures[1], id2);
  ASSERT_EQ(results_db.properties[0], id3);
  ASSERT_EQ(results_db.properties[1], id4);
  ASSERT_EQ(results_db.elementarySteps[0], id5);
  ASSERT_EQ(results_db.elementarySteps[1], id6);

  results2.structures.push_back(id1);
  results2.structures.push_back(id7);
  results2.properties.push_back(id3);
  results2.properties.push_back(id8);
  results2.elementarySteps.push_back(id5);
  results2.elementarySteps.push_back(id9);
  calc2.setResults(results2);
  auto results_db2 = calc2.getResults();
  ASSERT_EQ(results_db2.structures.size(), 2);
  ASSERT_EQ(results_db2.properties.size(), 2);
  ASSERT_EQ(results_db2.elementarySteps.size(), 2);
  ASSERT_EQ(results_db2.structures[0], id1);
  ASSERT_EQ(results_db2.structures[1], id7);
  ASSERT_EQ(results_db2.properties[0], id3);
  ASSERT_EQ(results_db2.properties[1], id8);
  ASSERT_EQ(results_db2.elementarySteps[0], id5);
  ASSERT_EQ(results_db2.elementarySteps[1], id9);

  auto combinedResults = results_db + results_db2;
  ASSERT_EQ(combinedResults.structures.size(), 3);
  ASSERT_EQ(combinedResults.properties.size(), 3);
  ASSERT_EQ(combinedResults.elementarySteps.size(), 3);
  ASSERT_EQ(combinedResults.structures[0], id1);
  ASSERT_EQ(combinedResults.structures[1], id2);
  ASSERT_EQ(combinedResults.structures[2], id7);
  ASSERT_EQ(combinedResults.properties[0], id3);
  ASSERT_EQ(combinedResults.properties[1], id4);
  ASSERT_EQ(combinedResults.properties[2], id8);
  ASSERT_EQ(combinedResults.elementarySteps[0], id5);
  ASSERT_EQ(combinedResults.elementarySteps[1], id6);
  ASSERT_EQ(combinedResults.elementarySteps[2], id9);

  calc.clearResults();
  results_db = calc.getResults();
  ASSERT_EQ(results_db.structures.size(), 0);
  ASSERT_EQ(results_db.properties.size(), 0);
  ASSERT_EQ(results_db.elementarySteps.size(), 0);
}

TEST_F(CalculationTest, ResultsFailCollection) {
  Calculation calc;
  Calculation::Results results;
  ASSERT_THROW(calc.getResults(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setResults(results), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.clearResults(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, ResultsFailID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  Calculation::Results results;
  ASSERT_THROW(calc.getResults(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setResults(results), Exceptions::MissingIDException);
  ASSERT_THROW(calc.clearResults(), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, Auxiliaries) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation::Job job("GeoOpt");
  Model model("dft", "pbe", "def2-svp");
  Calculation calc = Calculation::create(model, job, {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Auxiliaries Functionalities
  ID id1, id2, id3, id4, id5;
  calc.setAuxiliary("foo", id1);
  calc.setAuxiliary("bar", id2);
  calc.setAuxiliary("foobar", id3);
  calc.setAuxiliary("foobar", id2);
  ASSERT_TRUE(calc.hasAuxiliary("foo"));
  std::map<std::string, ID> aux = {{"foo", id3}, {"bar", id4}, {"barfoo", id5}};
  calc.removeAuxiliary("foo");
  ASSERT_FALSE(calc.hasAuxiliary("foo"));
  calc.setAuxiliaries(aux);
  ASSERT_TRUE(calc.hasAuxiliary("foo"));
  ASSERT_EQ(calc.getAuxiliary("foo"), id3);
  ASSERT_FALSE(calc.hasAuxiliary("foobar"));
  calc.clearAuxiliaries();
  ASSERT_FALSE(calc.hasAuxiliary("foo"));
  ASSERT_FALSE(calc.hasAuxiliary("bar"));
  ASSERT_FALSE(calc.hasAuxiliary("foobar"));
  ASSERT_FALSE(calc.hasAuxiliary("barfoo"));
}

TEST_F(CalculationTest, AuxiliariesFailCollection) {
  Calculation calc;
  ID id;
  ASSERT_THROW(calc.getAuxiliaries(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.getAuxiliary("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.hasAuxiliary("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.removeAuxiliary("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setAuxiliary("foo", id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setAuxiliaries(std::map<std::string, ID>{}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.clearAuxiliaries(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, AuxiliariesFailID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ID id;
  ASSERT_THROW(calc.getAuxiliaries(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.getAuxiliary("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.hasAuxiliary("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.removeAuxiliary("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setAuxiliary("foo", id), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setAuxiliaries(std::map<std::string, ID>{}), Exceptions::MissingIDException);
  ASSERT_THROW(calc.clearAuxiliaries(), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, RestartInformation) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation::Job job("GeoOpt");
  Model model("dft", "pbe", "def2-svp");
  Calculation calc = Calculation::create(model, job, {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // RestartInformation Functionalities
  ID id1, id2, id3, id4, id5;
  calc.setRestartInformation("foo", id1);
  calc.setRestartInformation("bar", id2);
  calc.setRestartInformation("foobar", id3);
  calc.setRestartInformation("foobar", id2);
  ASSERT_TRUE(calc.hasRestartInformation("foo"));
  std::map<std::string, ID> info = {{"foo", id3}, {"bar", id4}, {"barfoo", id5}};
  calc.removeRestartInformation("foo");
  ASSERT_FALSE(calc.hasRestartInformation("foo"));
  calc.setRestartInformation(info);
  ASSERT_TRUE(calc.hasRestartInformation("foo"));
  ASSERT_EQ(calc.getRestartInformation("foo"), id3);
  ASSERT_FALSE(calc.hasRestartInformation("foobar"));
  calc.clearRestartInformation();
  ASSERT_FALSE(calc.hasRestartInformation("foo"));
  ASSERT_FALSE(calc.hasRestartInformation("bar"));
  ASSERT_FALSE(calc.hasRestartInformation("foobar"));
  ASSERT_FALSE(calc.hasRestartInformation("barfoo"));
}

TEST_F(CalculationTest, RestartInformationFailCollection) {
  Calculation calc;
  ID id;
  ASSERT_THROW(calc.getRestartInformation(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.getRestartInformation("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.hasRestartInformation("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.removeRestartInformation("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setRestartInformation("foo", id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setRestartInformation(std::map<std::string, ID>{}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.clearRestartInformation(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, RestartInformationFailID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ID id;
  ASSERT_THROW(calc.getRestartInformation(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.getRestartInformation("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.hasRestartInformation("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.removeRestartInformation("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setRestartInformation("foo", id), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setRestartInformation(std::map<std::string, ID>{}), Exceptions::MissingIDException);
  ASSERT_THROW(calc.clearRestartInformation(), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, Output) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation calc =
      Calculation::create(Model("dft", "pbe", "def2-svp", "restricted"), Calculation::Job("geo_opt"), {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Raw output Functionalities
  ASSERT_FALSE(calc.hasRawOutput());
  auto output = calc.getRawOutput();
  ASSERT_EQ(output, "");

  calc.setRawOutput("wubba lubba dub dub");
  ASSERT_TRUE(calc.hasRawOutput());
  output = calc.getRawOutput();
  ASSERT_EQ(output, "wubba lubba dub dub");

  calc.clearRawOutput();
  ASSERT_FALSE(calc.hasRawOutput());
  output = calc.getRawOutput();
  ASSERT_EQ(output, "");
}

TEST_F(CalculationTest, OutputFailsCollection) {
  Calculation calc;
  ASSERT_THROW(calc.hasRawOutput(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.getRawOutput(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setRawOutput("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.clearRawOutput(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, OutputFailsID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ASSERT_THROW(calc.hasRawOutput(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.getRawOutput(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setRawOutput("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.clearRawOutput(), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, Comment) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation calc = Calculation::create(Model("dft", "pbe", "def2-svp"), Calculation::Job("geo_opt"), {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Comment Functionalities
  ASSERT_FALSE(calc.hasComment());
  auto comment = calc.getComment();
  ASSERT_EQ(comment, "");

  calc.setComment("wubba lubba dub dub");
  ASSERT_TRUE(calc.hasComment());
  comment = calc.getComment();
  ASSERT_EQ(comment, "wubba lubba dub dub");

  calc.clearComment();
  ASSERT_FALSE(calc.hasComment());
  comment = calc.getComment();
  ASSERT_EQ(comment, "");
}

TEST_F(CalculationTest, CommentFailsCollection) {
  Calculation calc;
  ASSERT_THROW(calc.hasComment(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.getComment(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setComment("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.clearComment(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, CommentFailsID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ASSERT_THROW(calc.hasComment(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.getComment(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setComment("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.clearComment(), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, Executor) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation calc = Calculation::create(Model("dft", "pbe", "def2-svp"), Calculation::Job("geo_opt"), {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Executor Functionalities
  ASSERT_FALSE(calc.hasExecutor());
  auto executor = calc.getExecutor();
  ASSERT_EQ(executor, "");

  calc.setExecutor("wubba lubba dub dub");
  ASSERT_TRUE(calc.hasExecutor());
  executor = calc.getExecutor();
  ASSERT_EQ(executor, "wubba lubba dub dub");

  calc.clearExecutor();
  ASSERT_FALSE(calc.hasExecutor());
  executor = calc.getExecutor();
  ASSERT_EQ(executor, "");
}

TEST_F(CalculationTest, ExecutorFailsCollection) {
  Calculation calc;
  ASSERT_THROW(calc.hasExecutor(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.getExecutor(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setExecutor("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.clearExecutor(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, ExecutorFailsID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ASSERT_THROW(calc.hasExecutor(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.getExecutor(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setExecutor("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(calc.clearExecutor(), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, Runtime) {
  // Setup
  ID s1, s2, s3;
  auto coll = db.getCollection("calculations");
  Calculation calc = Calculation::create(Model("dft", "pbe", "def2-svp"), Calculation::Job("geo_opt"), {s1, s2, s3}, coll);
  ASSERT_TRUE(calc.hasId());

  // Runtime Functionalities
  ASSERT_FALSE(calc.hasRuntime());
  ASSERT_THROW(calc.getRuntime(), Exceptions::MissingIdOrField);

  calc.setRuntime(125.0125);
  ASSERT_TRUE(calc.hasRuntime());
  auto runtime = calc.getRuntime();
  ASSERT_EQ(runtime, 125.0125);

  calc.clearRuntime();
  ASSERT_FALSE(calc.hasRuntime());
  ASSERT_THROW(calc.getRuntime(), Exceptions::MissingIdOrField);
}

TEST_F(CalculationTest, RuntimeFailsCollection) {
  Calculation calc;
  ASSERT_THROW(calc.hasRuntime(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.getRuntime(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.setRuntime(1.0), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(calc.clearRuntime(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CalculationTest, RuntimeFailsID) {
  auto coll = db.getCollection("calculations");
  Calculation calc;
  calc.link(coll);
  ASSERT_THROW(calc.hasRuntime(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.getRuntime(), Exceptions::MissingIDException);
  ASSERT_THROW(calc.setRuntime(1.0), Exceptions::MissingIDException);
  ASSERT_THROW(calc.clearRuntime(), Exceptions::MissingIDException);
}

TEST_F(CalculationTest, LargeResultList) {
  ID s1;
  auto coll = db.getCollection("calculations");
  Calculation calc =
      Calculation::create(Model("dft", "pbe", "def2-svp", "restricted"), Calculation::Job("geo_opt"), {s1}, coll);
  auto results = calc.getResults();
  std::vector<ID> large_id_list(60000);
  calc.setStatus(CalculationStatus::COMPLETE);
  results.elementarySteps = large_id_list;
  results.properties = large_id_list;
  results.structures = large_id_list;
  ASSERT_NO_THROW(calc.setResults(results));
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
