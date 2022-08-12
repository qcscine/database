/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Calculation.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/NumberProperty.h>
#include <Database/Objects/Structure.h>
#include <Utils/Geometry/AtomCollection.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class StructureTest StructureTest.cpp
 * @brief Comprises tests for the class Scine::Database::Structure.
 * @test
 */
class StructureTest : public Test {
 public:
  StructureTest() {
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
Credentials StructureTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_StructureTest");
Manager StructureTest::db;

TEST_F(StructureTest, Create1) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  auto coll = db.getCollection("structures");
  ASSERT_NO_THROW(Structure::create(atoms, 0, 1, coll));
}

TEST_F(StructureTest, Create2) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  ASSERT_NO_THROW(Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll));
}

TEST_F(StructureTest, Atoms) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);

  Utils::AtomCollection atoms2;
  atoms2.resize(4);
  atoms2.setElement(3, Utils::ElementType::He);
  atoms2.setElement(2, Utils::ElementType::Ar);
  atoms2.setElement(1, Utils::ElementType::Mo);
  atoms2.setElement(0, Utils::ElementType::Te);
  atoms2.setPosition(3, Eigen::Vector3d(1, 2, 3));
  atoms2.setPosition(2, Eigen::Vector3d(4, 5, 6));
  atoms2.setPosition(1, Eigen::Vector3d(0, 0, 0));
  atoms2.setPosition(0, Eigen::Vector3d(9, 9, 9));
  ASSERT_EQ(2, structure.hasAtoms());
  auto ret1 = structure.getAtoms();
  ASSERT_EQ(ret1.getElement(0), Utils::ElementType::H);
  ASSERT_EQ(ret1.getElement(1), Utils::ElementType::H);
  structure.setAtoms(atoms2);
  ASSERT_EQ(4, structure.hasAtoms());
  auto ret2 = structure.getAtoms();
  ASSERT_EQ(ret2.getElement(3), Utils::ElementType::He);
  ASSERT_EQ(ret2.getElement(2), Utils::ElementType::Ar);
  ASSERT_EQ(ret2.getElement(1), Utils::ElementType::Mo);
  ASSERT_EQ(ret2.getElement(0), Utils::ElementType::Te);
  structure.clearAtoms();
  ASSERT_EQ(0, structure.hasAtoms());
}

TEST_F(StructureTest, AtomsFails1) {
  Structure structure;
  Utils::AtomCollection atoms;
  ASSERT_THROW(structure.setAtoms(atoms), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getAtoms(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.hasAtoms(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.clearAtoms(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, AtomsFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  Utils::AtomCollection atoms;
  ASSERT_THROW(structure.setAtoms(atoms), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getAtoms(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.hasAtoms(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.clearAtoms(), Exceptions::MissingIDException);
}

TEST_F(StructureTest, Charge) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  ASSERT_TRUE(structure.hasId());
  ASSERT_EQ(0, structure.getCharge());
  structure.setCharge(5);
  ASSERT_EQ(5, structure.getCharge());
  structure.setCharge(-999);
  ASSERT_EQ(-999, structure.getCharge());
}

TEST_F(StructureTest, ChargeFails1) {
  Structure structure;
  ASSERT_THROW(structure.setCharge(1), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getCharge(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, ChargeFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  ASSERT_THROW(structure.setCharge(1), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getCharge(), Exceptions::MissingIDException);
}

TEST_F(StructureTest, Multiplicity) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  ASSERT_TRUE(structure.hasId());
  ASSERT_EQ(1, structure.getMultiplicity());
  structure.setMultiplicity(5);
  ASSERT_EQ(5, structure.getMultiplicity());
  structure.setMultiplicity(-999);
  ASSERT_EQ(-999, structure.getMultiplicity());
}

TEST_F(StructureTest, MultiplicityFails1) {
  Structure structure;
  ASSERT_THROW(structure.setMultiplicity(1), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getMultiplicity(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, MultiplicityFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  ASSERT_THROW(structure.setMultiplicity(1), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getMultiplicity(), Exceptions::MissingIDException);
}

TEST_F(StructureTest, Model) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  ASSERT_TRUE(structure.hasId());
  auto ret1 = structure.getModel();
  ASSERT_EQ(ret1.basisSet, model.basisSet);
  ASSERT_EQ(ret1.temperature, model.temperature);
  ASSERT_EQ(ret1.electronicTemperature, model.electronicTemperature);
  ASSERT_EQ(ret1.method, model.method);
  ASSERT_EQ(ret1.program, model.program);
  Model model2("am1", "am1", "");
  model2.temperature = "278.0";
  model2.electronicTemperature = "5.0";
  model2.program = "sparrow";
  structure.setModel(model2);
  auto ret2 = structure.getModel();
  ASSERT_EQ(ret2.basisSet, model2.basisSet);
  ASSERT_EQ(ret2.temperature, model2.temperature);
  ASSERT_EQ(ret2.electronicTemperature, model2.electronicTemperature);
  ASSERT_EQ(ret2.method, model2.method);
  ASSERT_EQ(ret2.program, model2.program);
}

TEST_F(StructureTest, ModelFails1) {
  Structure structure;
  Model model("dft", "pbe", "def2-svp");
  ASSERT_THROW(structure.setModel(model), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getModel(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, ModelFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  Model model("dft", "pbe", "def2-svp");
  ASSERT_THROW(structure.setModel(model), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getModel(), Exceptions::MissingIDException);
}

TEST_F(StructureTest, Label) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  ASSERT_EQ(Structure::LABEL::MINIMUM_GUESS, structure.getLabel());
  structure.setLabel(Structure::LABEL::NONE);
  ASSERT_EQ(Structure::LABEL::NONE, structure.getLabel());
  structure.setLabel(Structure::LABEL::TS_GUESS);
  ASSERT_EQ(Structure::LABEL::TS_GUESS, structure.getLabel());
}

TEST_F(StructureTest, LabelFails1) {
  Structure structure;
  ASSERT_THROW(structure.setLabel(Structure::LABEL::NONE), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getLabel(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, LabelFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  ASSERT_THROW(structure.setLabel(Structure::LABEL::NONE), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getLabel(), Exceptions::MissingIDException);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
TEST_F(StructureTest, Compound) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  ASSERT_TRUE(structure.hasId());
  ASSERT_FALSE(structure.hasCompound());
  ID id2;
  structure.setCompound(id2);
  ASSERT_TRUE(structure.hasCompound());
  ASSERT_EQ(id2, structure.getCompound());
  structure.clearCompound();
  ASSERT_FALSE(structure.hasCompound());
}

TEST_F(StructureTest, CompoundFails1) {
  Structure structure;
  ID id;
  ASSERT_THROW(structure.setCompound(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getCompound(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.hasCompound(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.clearCompound(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, CompoundFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  ID id;
  ASSERT_THROW(structure.setCompound(id), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getCompound(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.hasCompound(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.clearCompound(), Exceptions::MissingIDException);
}
#pragma GCC diagnostic pop

TEST_F(StructureTest, Aggregate) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  ASSERT_TRUE(structure.hasId());
  ASSERT_FALSE(structure.hasAggregate());
  ID id2;
  structure.setAggregate(id2);
  ASSERT_TRUE(structure.hasAggregate());
  ASSERT_EQ(id2, structure.getAggregate());
  structure.clearAggregate();
  ASSERT_FALSE(structure.hasAggregate());
}

TEST_F(StructureTest, AggregateFails1) {
  Structure structure;
  ID id;
  ASSERT_THROW(structure.setAggregate(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getAggregate(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.hasAggregate(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.clearAggregate(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, AggregateFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  ID id;
  ASSERT_THROW(structure.setAggregate(id), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getAggregate(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.hasAggregate(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.clearAggregate(), Exceptions::MissingIDException);
}

TEST_F(StructureTest, Graph) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  // Checks
  ASSERT_EQ(0, structure.hasGraphs());
  ASSERT_FALSE(structure.hasGraph("FROWN"));
  structure.setGraph("FROWN", "HHC=CHH");
  ASSERT_EQ(1, structure.hasGraphs());
  ASSERT_TRUE(structure.hasGraph("FROWN"));
  ASSERT_EQ("HHC=CHH", structure.getGraph("FROWN"));
  structure.removeGraph("FROWN");
  ASSERT_EQ(0, structure.hasGraphs());
  ASSERT_FALSE(structure.hasGraph("FROWN"));
  std::map<std::string, std::string> graphs = {{"FROWN", "ABCDEF"}, {"SMILES", "FEDCBA"}};
  structure.setGraphs(graphs);
  ASSERT_EQ(2, structure.hasGraphs());
  ASSERT_TRUE(structure.hasGraph("FROWN"));
  auto ret = structure.getGraphs();
  ASSERT_EQ(ret.at("FROWN"), graphs.at("FROWN"));
  ASSERT_EQ(ret.at("SMILES"), graphs.at("SMILES"));
  structure.clearGraphs();
  ASSERT_EQ(0, structure.hasGraphs());
  ASSERT_FALSE(structure.hasGraph("FROWN"));
}

TEST_F(StructureTest, GraphFails1) {
  Structure structure;
  ASSERT_THROW(structure.setGraph("FROWN", "HHC=CHH"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getGraph("FROWN"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.hasGraph("FROWN"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.setGraphs({}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getGraphs(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.hasGraphs(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.removeGraph("FROWN"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.clearGraphs(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, GraphFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  ASSERT_THROW(structure.setGraph("FROWN", "HHC=CHH"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getGraph("FROWN"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.hasGraph("FROWN"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.setGraphs({}), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getGraphs(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.hasGraphs(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.removeGraph("FROWN"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.clearGraphs(), Exceptions::MissingIDException);
}

TEST_F(StructureTest, Comment) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  ASSERT_TRUE(structure.hasId());
  ASSERT_FALSE(structure.hasComment());
  structure.setComment("foo");
  ASSERT_TRUE(structure.hasComment());
  ASSERT_EQ("foo", structure.getComment());
  structure.clearComment();
  ASSERT_FALSE(structure.hasComment());
}

TEST_F(StructureTest, CommentFails1) {
  Structure structure;
  std::string s = "bla";
  ASSERT_THROW(structure.setComment(s), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getComment(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.hasComment(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.clearComment(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, CommentFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  std::string s = "bla";
  ASSERT_THROW(structure.setComment(s), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getComment(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.hasComment(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.clearComment(), Exceptions::MissingIDException);
}

TEST_F(StructureTest, Property1) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  // Checks
  ID id1, id2;
  ASSERT_FALSE(structure.hasProperty("hessian"));
  ASSERT_FALSE(structure.hasProperty(id1));
  structure.setProperty("hessian", id1);
  ASSERT_TRUE(structure.hasProperty("hessian"));
  ASSERT_TRUE(structure.hasProperty(id1));
  ASSERT_FALSE(structure.hasProperty(id2));
  ASSERT_EQ(id1, structure.getProperty("hessian"));
  structure.addProperty("hessian", id2);
  ASSERT_TRUE(structure.hasProperty(id2));
  ASSERT_THROW(structure.getProperty("hessian"), Exceptions::FieldException);
  structure.removeProperty("hessian", id1);
  ASSERT_TRUE(structure.hasProperty(id2));
  ASSERT_FALSE(structure.hasProperty(id1));
}

TEST_F(StructureTest, Property2) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  // Checks
  ID id1, id2, id3;
  ASSERT_EQ(0, structure.hasProperties("hessian"));
  structure.setProperties("hessian", {id1, id2});
  ASSERT_EQ(2, structure.hasProperties("hessian"));
  ASSERT_EQ(id1, structure.getProperties("hessian")[0]);
  ASSERT_EQ(id2, structure.getProperties("hessian")[1]);
  structure.setProperties("hessian", {id3});
  ASSERT_EQ(id3, structure.getProperties("hessian")[0]);
  ASSERT_EQ(1, structure.hasProperties("hessian"));
  structure.clearProperties("hessian");
  ASSERT_EQ(0, structure.hasProperties("hessian"));
}

TEST_F(StructureTest, Property3) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  // Checks
  ID id1, id2, id3, id4, id5, id6, id7, id8, id9;
  std::map<std::string, std::vector<ID>> prop1 = {
      {"electronic_energy", {id1, id2, id3}}, {"hessian", {id4, id5}}, {"gradients", {id8, id9}}};
  std::map<std::string, std::vector<ID>> prop2 = {
      {"mulliken_charges", {id6, id7}},
      {"gradients", {id8, id9}},
  };
  ASSERT_EQ(0, structure.hasProperties("electronic_energy"));
  ASSERT_EQ(0, structure.hasProperties("mulliken_charges"));
  ASSERT_EQ(0, structure.hasProperties("hessian"));
  ASSERT_EQ(0, structure.hasProperties("gradients"));
  structure.setAllProperties(prop1);
  auto ret1 = structure.getAllProperties();
  ASSERT_EQ(prop1.at("electronic_energy")[0], ret1.at("electronic_energy")[0]);
  ASSERT_EQ(prop1.at("electronic_energy")[2], ret1.at("electronic_energy")[2]);
  ASSERT_EQ(prop1.at("hessian")[0], ret1.at("hessian")[0]);
  ASSERT_EQ(prop1.at("hessian")[1], ret1.at("hessian")[1]);
  ASSERT_EQ(prop1.at("gradients")[0], ret1.at("gradients")[0]);
  structure.setAllProperties(prop2);
  auto ret2 = structure.getAllProperties();
  ASSERT_EQ(prop2.at("mulliken_charges")[0], ret2.at("mulliken_charges")[0]);
  ASSERT_EQ(prop2.at("mulliken_charges")[1], ret2.at("mulliken_charges")[1]);
  ASSERT_EQ(prop2.at("gradients")[0], ret2.at("gradients")[0]);
  ASSERT_EQ(prop2.at("gradients")[1], ret2.at("gradients")[1]);
  structure.clearAllProperties();
  ASSERT_EQ(0, structure.hasProperties("electronic_energy"));
  ASSERT_EQ(0, structure.hasProperties("mulliken_charges"));
  ASSERT_EQ(0, structure.hasProperties("hessian"));
  ASSERT_EQ(0, structure.hasProperties("gradients"));
}

TEST_F(StructureTest, Property4) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model1("dft", "pbe", "def2-svp");
  model1.program = "serenity";
  Model model2("am1", "am1", "");
  model2.program = "sparrow";
  auto structures = db.getCollection("structures");
  auto properties = db.getCollection("properties");
  Structure structure = Structure::create(atoms, 0, 1, model1, Structure::LABEL::MINIMUM_GUESS, structures);
  const ID& id = structure.id();
  ID calculation;
  NumberProperty p1 = NumberProperty::create("electronic_energy", model1, 1.0, id, calculation, properties);
  NumberProperty p2 = NumberProperty::create("electronic_energy", model2, 2.0, id, calculation, properties);
  NumberProperty p3 = NumberProperty::create("electronic_energy", model2, 3.0, id, calculation, properties);
  structure.addProperty("electronic_energy", p1.id());
  structure.addProperty("electronic_energy", p2.id());
  structure.addProperty("electronic_energy", p3.id());
  // Checks
  Model none("dft", "sdfasdf", "asdf");
  auto ret0 = structure.queryProperties("electronic_energy", none, properties);
  EXPECT_EQ(0, ret0.size());
  auto ret1 = structure.queryProperties("electronic_energy", model1, properties);
  EXPECT_EQ(1, ret1.size());
  auto ret2 = structure.queryProperties("electronic_energy", model2, properties);
  EXPECT_EQ(2, ret2.size());
  Model anydft("dft", "any", "any");
  auto ret3 = structure.queryProperties("electronic_energy", anydft, properties);
  EXPECT_EQ(1, ret3.size());
  Model any("any", "any", "any");
  auto ret4 = structure.queryProperties("electronic_energy", any, properties);
  // MB: I changed the reference from 3 to 1 because the field "any" should exclude "none" in the model definition.
  // This interpretation is in line with the operator== in Model.cpp/.h
  EXPECT_EQ(1, ret4.size());
  auto ret5 = structure.queryProperties("none_existing_key", none, properties);
  EXPECT_EQ(0, ret5.size());
}

TEST_F(StructureTest, PropertyFails1) {
  auto coll = db.getCollection("structures");
  Structure structure;
  ID id;
  Model model("dft", "pbe", "def2-svp");
  ASSERT_THROW(structure.hasProperty("key"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.hasProperty(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getProperty("key"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.setProperty("key", id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.addProperty("key", id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.removeProperty("key", id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.setProperties("key", {}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getProperties("key"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.hasProperties("key"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.clearProperties("key"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.queryProperties("key", model, coll), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getAllProperties(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.setAllProperties({{"key", {id}}}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.clearAllProperties(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, PropertyFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  ID id;
  Model model("dft", "pbe", "def2-svp");
  ASSERT_THROW(structure.hasProperty("key"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.hasProperty(id), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getProperty("key"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.setProperty("key", id), Exceptions::MissingIDException);
  ASSERT_THROW(structure.addProperty("key", id), Exceptions::MissingIDException);
  ASSERT_THROW(structure.removeProperty("key", id), Exceptions::MissingIDException);
  ASSERT_THROW(structure.setProperties("key", {}), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getProperties("key"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.hasProperties("key"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.clearProperties("key"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.queryProperties("key", model, coll), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getAllProperties(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.setAllProperties({{"key", {id}}}), Exceptions::MissingIDException);
  ASSERT_THROW(structure.clearAllProperties(), Exceptions::MissingIDException);
}

TEST_F(StructureTest, Calculation1) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  // Checks
  ID id1, id2;
  ASSERT_FALSE(structure.hasCalculation("hessian"));
  ASSERT_FALSE(structure.hasCalculation(id1));
  structure.setCalculation("hessian", id1);
  ASSERT_TRUE(structure.hasCalculation("hessian"));
  ASSERT_TRUE(structure.hasCalculation(id1));
  ASSERT_FALSE(structure.hasCalculation(id2));
  ASSERT_EQ(id1, structure.getCalculation("hessian"));
  structure.addCalculation("hessian", id2);
  ASSERT_TRUE(structure.hasCalculation(id2));
  ASSERT_THROW(structure.getCalculation("hessian"), Exceptions::FieldException);
  structure.removeCalculation("hessian", id1);
  ASSERT_TRUE(structure.hasCalculation(id2));
  ASSERT_FALSE(structure.hasCalculation(id1));
}

TEST_F(StructureTest, Calculation2) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  // Checks
  ID id1, id2, id3;
  ASSERT_EQ(0, structure.hasCalculations("hessian"));
  structure.setCalculations("hessian", {id1, id2});
  ASSERT_EQ(2, structure.hasCalculations("hessian"));
  ASSERT_EQ(id1, structure.getCalculations("hessian")[0]);
  ASSERT_EQ(id2, structure.getCalculations("hessian")[1]);
  structure.setCalculations("hessian", {id3});
  ASSERT_EQ(id3, structure.getCalculations("hessian")[0]);
  ASSERT_EQ(1, structure.hasCalculations("hessian"));
  structure.clearCalculations("hessian");
  ASSERT_EQ(0, structure.hasCalculations("hessian"));
}

TEST_F(StructureTest, Calculation3) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model("dft", "pbe", "def2-svp");
  auto coll = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model, Structure::LABEL::MINIMUM_GUESS, coll);
  // Checks
  ID id1, id2, id3, id4, id5, id6, id7, id8, id9;
  std::map<std::string, std::vector<ID>> calc1 = {
      {"electronic_energy", {id1, id2, id3}}, {"hessian", {id4, id5}}, {"gradients", {id8, id9}}};
  std::map<std::string, std::vector<ID>> calc2 = {
      {"mulliken_charges", {id6, id7}},
      {"gradients", {id8, id9}},
  };
  ASSERT_EQ(0, structure.hasCalculations("electronic_energy"));
  ASSERT_EQ(0, structure.hasCalculations("mulliken_charges"));
  ASSERT_EQ(0, structure.hasCalculations("hessian"));
  ASSERT_EQ(0, structure.hasCalculations("gradients"));
  structure.setAllCalculations(calc1);
  auto ret1 = structure.getAllCalculations();
  ASSERT_EQ(calc1.at("electronic_energy")[0], ret1.at("electronic_energy")[0]);
  ASSERT_EQ(calc1.at("electronic_energy")[2], ret1.at("electronic_energy")[2]);
  ASSERT_EQ(calc1.at("hessian")[0], ret1.at("hessian")[0]);
  ASSERT_EQ(calc1.at("hessian")[1], ret1.at("hessian")[1]);
  ASSERT_EQ(calc1.at("gradients")[0], ret1.at("gradients")[0]);
  structure.setAllCalculations(calc2);
  auto ret2 = structure.getAllCalculations();
  ASSERT_EQ(calc2.at("mulliken_charges")[0], ret2.at("mulliken_charges")[0]);
  ASSERT_EQ(calc2.at("mulliken_charges")[1], ret2.at("mulliken_charges")[1]);
  ASSERT_EQ(calc2.at("gradients")[0], ret2.at("gradients")[0]);
  ASSERT_EQ(calc2.at("gradients")[1], ret2.at("gradients")[1]);
  structure.clearAllCalculations();
  ASSERT_EQ(0, structure.hasCalculations("electronic_energy"));
  ASSERT_EQ(0, structure.hasCalculations("mulliken_charges"));
  ASSERT_EQ(0, structure.hasCalculations("hessian"));
  ASSERT_EQ(0, structure.hasCalculations("gradients"));
}

TEST_F(StructureTest, Calculation4) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model1("dft", "pbe", "def2-svp");
  model1.program = "serenity";
  Model model2("am1", "am1", "");
  model2.program = "sparrow";
  auto structures = db.getCollection("structures");
  auto calculations = db.getCollection("calculations");
  Structure structure = Structure::create(atoms, 0, 1, model1, Structure::LABEL::MINIMUM_GUESS, structures);
  const ID& id = structure.id();
  Calculation::Job job("test");
  Calculation c1 = Calculation::create(model1, job, {id}, calculations);
  Calculation c2 = Calculation::create(model2, job, {id}, calculations);
  Calculation c3 = Calculation::create(model2, job, {id}, calculations);
  structure.addCalculation(job.order, c1.id());
  structure.addCalculation(job.order, c2.id());
  structure.addCalculation(job.order, c3.id());
  // Checks
  Model none("dft", "sdfasdf", "asdf");
  auto ret0 = structure.queryCalculations(job.order, none, calculations);
  EXPECT_EQ(0, ret0.size());
  auto ret1 = structure.queryCalculations(job.order, model1, calculations);
  EXPECT_EQ(1, ret1.size());
  auto ret2 = structure.queryCalculations(job.order, model2, calculations);
  EXPECT_EQ(2, ret2.size());
  Model anydft("dft", "any", "any");
  auto ret3 = structure.queryCalculations(job.order, anydft, calculations);
  EXPECT_EQ(1, ret3.size());
  Model any("any", "any", "any");
  auto ret4 = structure.queryCalculations(job.order, any, calculations);
  EXPECT_EQ(1, ret4.size());
  auto ret5 = structure.queryCalculations("none_existing_key", none, calculations);
  EXPECT_EQ(0, ret5.size());
}

TEST_F(StructureTest, Calculation5) {
  // Setup
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model1("dft", "pbe", "def2-svp");
  model1.program = "serenity";
  Model model2("am1", "am1", "");
  model2.program = "sparrow";
  auto structures = db.getCollection("structures");
  auto calculations = db.getCollection("calculations");
  Structure structure = Structure::create(atoms, 0, 1, model1, Structure::LABEL::MINIMUM_GUESS, structures);
  const ID& id = structure.id();
  Calculation::Job job("test");
  Calculation c1 = Calculation::create(model1, job, {id}, calculations);
  Calculation c2 = Calculation::create(model2, job, {id}, calculations);
  Calculation c3 = Calculation::create(model2, job, {id}, calculations);
  structure.addCalculations(job.order, {c1.id(), c2.id(), c3.id()});
  // Checks
  Model none("dft", "sdfasdf", "asdf");
  auto ret0 = structure.queryCalculations(job.order, none, calculations);
  EXPECT_EQ(0, ret0.size());
  auto ret1 = structure.queryCalculations(job.order, model1, calculations);
  EXPECT_EQ(1, ret1.size());
  auto ret2 = structure.queryCalculations(job.order, model2, calculations);
  EXPECT_EQ(2, ret2.size());
  Model anydft("dft", "any", "any");
  auto ret3 = structure.queryCalculations(job.order, anydft, calculations);
  EXPECT_EQ(1, ret3.size());
  Model any("any", "any", "any");
  auto ret4 = structure.queryCalculations(job.order, any, calculations);
  EXPECT_EQ(1, ret4.size());
  auto ret5 = structure.queryCalculations("none_existing_key", none, calculations);
  EXPECT_EQ(0, ret5.size());
}

TEST_F(StructureTest, CalculationFails1) {
  auto coll = db.getCollection("structures");
  Structure structure;
  ID id;
  Model model("dft", "pbe", "def2-svp");
  ASSERT_THROW(structure.hasCalculation("key"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.hasCalculation(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getCalculation("key"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.setCalculation("key", id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.addCalculation("key", id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.removeCalculation("key", id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.setCalculations("key", {}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getCalculations("key"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.hasCalculations("key"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.clearCalculations("key"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.queryCalculations("key", model, coll), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.getAllCalculations(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.setAllCalculations({{"key", {id}}}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(structure.clearAllCalculations(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(StructureTest, CalculationFails2) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  ID id;
  Model model("dft", "pbe", "def2-svp");
  ASSERT_THROW(structure.hasCalculation("key"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.hasCalculation(id), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getCalculation("key"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.setCalculation("key", id), Exceptions::MissingIDException);
  ASSERT_THROW(structure.addCalculation("key", id), Exceptions::MissingIDException);
  ASSERT_THROW(structure.removeCalculation("key", id), Exceptions::MissingIDException);
  ASSERT_THROW(structure.setCalculations("key", {}), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getCalculations("key"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.hasCalculations("key"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.clearCalculations("key"), Exceptions::MissingIDException);
  ASSERT_THROW(structure.queryCalculations("key", model, coll), Exceptions::MissingIDException);
  ASSERT_THROW(structure.getAllCalculations(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.setAllCalculations({{"key", {id}}}), Exceptions::MissingIDException);
  ASSERT_THROW(structure.clearAllCalculations(), Exceptions::MissingIDException);
}

TEST_F(StructureTest, IsDuplicateOf) {
  Utils::AtomCollection atoms;
  atoms.resize(2);
  atoms.setElement(0, Utils::ElementType::H);
  atoms.setElement(1, Utils::ElementType::H);
  atoms.setPosition(0, Eigen::Vector3d(+1, 0, 0));
  atoms.setPosition(1, Eigen::Vector3d(-1, 0, 0));
  Model model1("dft", "pbe", "def2-svp");
  model1.program = "serenity";
  auto structures = db.getCollection("structures");
  Structure structure = Structure::create(atoms, 0, 1, model1, Structure::LABEL::MINIMUM_GUESS, structures);

  ID id;
  ID id2;

  ASSERT_THROW(structure.isDuplicateOf(), Exceptions::UnpopulatedObjectException);
  structure.setAsDuplicateOf(id);
  ASSERT_TRUE(structure.isDuplicateOf() == id);
  structure.setAsDuplicateOf(id2);
  ASSERT_TRUE(structure.isDuplicateOf() == id2);
  structure.clearDuplicateID();
  ASSERT_THROW(structure.isDuplicateOf(), Exceptions::UnpopulatedObjectException);
}

TEST_F(StructureTest, IsDuplicateOfFailuresID) {
  auto coll = db.getCollection("structures");
  Structure structure;
  structure.link(coll);
  ID id;

  ASSERT_THROW(structure.isDuplicateOf(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.clearDuplicateID(), Exceptions::MissingIDException);
  ASSERT_THROW(structure.setAsDuplicateOf(id), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
