/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/NumberProperty.h>
#include <Database/Objects/Property.h>
#include <Database/Objects/VectorProperty.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class PropertyTest
 * @brief Comprises tests for the class Scine::Database::Property.
 * @test
 */
struct PropertyTest : public Test {
  Credentials credentials{TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_PropertyTest"};
  Manager db;

  void SetUp() final {
    db.setCredentials(credentials);
    db.connect();
    db.init();
  }
  void TearDown() final {
    db.wipe();
  }
};

TEST_F(PropertyTest, PropertyName) {
  // Setup
  auto coll = db.getCollection("properties");
  NumberProperty test = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), 0.0, coll);

  Property property(test.id(), coll);
  ASSERT_TRUE(property.hasId());

  // PropertyName Functionalities
  ASSERT_EQ(property.getPropertyName(), "density_matrix");
  property.setPropertyName("electronic_energy");
  ASSERT_EQ(property.getPropertyName(), "electronic_energy");
}

TEST_F(PropertyTest, PropertyNameFailsCollection) {
  Property property;
  ASSERT_THROW(property.getPropertyName(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.setPropertyName("asdf"), Exceptions::MissingLinkedCollectionException);
}

TEST_F(PropertyTest, PropertyNameFailsID) {
  auto coll = db.getCollection("properties");
  Property property;
  property.link(coll);
  ASSERT_THROW(property.getPropertyName(), Exceptions::MissingIDException);
  ASSERT_THROW(property.setPropertyName("asdf"), Exceptions::MissingIDException);
}

TEST_F(PropertyTest, Model) {
  // Setup
  auto coll = db.getCollection("properties");

  auto property = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), 0.0, coll);
  ASSERT_TRUE(property.hasId());

  // Model Functionalities
  auto model = property.getModel();
  ASSERT_EQ(model.method, "pbe");
  ASSERT_EQ(model.methodFamily, "dft");
  ASSERT_EQ(model.basisSet, "def2-svp");
  ASSERT_EQ(model.spinMode, "any");
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
  model.methodFamily = "k";
  property.setModel(model);
  auto model_db = property.getModel();
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
  ASSERT_EQ(model.methodFamily, model_db.methodFamily);
}

TEST_F(PropertyTest, ModelFailsCollection) {
  Property property;
  ASSERT_THROW(property.getModel(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.setModel(Model("dft", "pbe", "def2-svp")), Exceptions::MissingLinkedCollectionException);
}

TEST_F(PropertyTest, ModelFailsID) {
  auto coll = db.getCollection("properties");
  Property property;
  property.link(coll);
  ASSERT_THROW(property.getModel(), Exceptions::MissingIDException);
  ASSERT_THROW(property.setModel(Model("dft", "pbe", "def2-svp")), Exceptions::MissingIDException);
}

TEST_F(PropertyTest, Structure) {
  // Setup
  auto coll = db.getCollection("properties");

  auto property = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), 0.0, coll);
  ASSERT_TRUE(property.hasId());

  // tests
  ID id;
  ASSERT_FALSE(property.hasStructure());
  property.setStructure(id);
  ASSERT_TRUE(property.hasStructure());
  ASSERT_EQ(property.getStructure(), id);
  property.clearStructure();
  ASSERT_FALSE(property.hasStructure());
}

TEST_F(PropertyTest, StructureFailsCollection) {
  Property property;
  ID id;
  ASSERT_THROW(property.hasStructure(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.setStructure(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.getStructure(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.clearStructure(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(PropertyTest, StructureFailsID) {
  auto coll = db.getCollection("properties");
  Property property;
  property.link(coll);
  ID id;
  ASSERT_THROW(property.hasStructure(), Exceptions::MissingIDException);
  ASSERT_THROW(property.setStructure(id), Exceptions::MissingIDException);
  ASSERT_THROW(property.getStructure(), Exceptions::MissingIDException);
  ASSERT_THROW(property.clearStructure(), Exceptions::MissingIDException);
}

TEST_F(PropertyTest, Calculation) {
  // Setup
  auto coll = db.getCollection("properties");
  auto property = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), 0.0, coll);
  ASSERT_TRUE(property.hasId());

  // tests
  ID id;
  ASSERT_FALSE(property.hasCalculation());
  property.setCalculation(id);
  ASSERT_TRUE(property.hasCalculation());
  ASSERT_EQ(property.getCalculation(), id);
  property.clearCalculation();
  ASSERT_FALSE(property.hasCalculation());
}

TEST_F(PropertyTest, CalculationFailsCollection) {
  Property property;
  ID id;
  ASSERT_THROW(property.getCalculation(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.setCalculation(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.hasCalculation(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.clearCalculation(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(PropertyTest, CalculationFailsID) {
  auto coll = db.getCollection("properties");
  Property property;
  property.link(coll);
  ID id;
  ASSERT_THROW(property.getCalculation(), Exceptions::MissingIDException);
  ASSERT_THROW(property.setCalculation(id), Exceptions::MissingIDException);
  ASSERT_THROW(property.hasCalculation(), Exceptions::MissingIDException);
  ASSERT_THROW(property.clearCalculation(), Exceptions::MissingIDException);
}

TEST_F(PropertyTest, Comment) {
  // Setup
  auto coll = db.getCollection("properties");
  auto property = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), 0.0, coll);
  ASSERT_TRUE(property.hasId());

  // Comment Functionalities
  ASSERT_FALSE(property.hasComment());
  auto comment = property.getComment();
  ASSERT_EQ(comment, "");

  property.setComment("wubba lubba dub dub");
  ASSERT_TRUE(property.hasComment());
  comment = property.getComment();
  ASSERT_EQ(comment, "wubba lubba dub dub");

  property.clearComment();
  ASSERT_FALSE(property.hasComment());
  comment = property.getComment();
  ASSERT_EQ(comment, "");
}

TEST_F(PropertyTest, CommentFailsCollection) {
  Property property;
  ASSERT_THROW(property.hasComment(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.getComment(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.setComment("foo"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(property.clearComment(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(PropertyTest, CommentFailsID) {
  auto coll = db.getCollection("properties");
  Property property;
  property.link(coll);
  ASSERT_THROW(property.hasComment(), Exceptions::MissingIDException);
  ASSERT_THROW(property.getComment(), Exceptions::MissingIDException);
  ASSERT_THROW(property.setComment("foo"), Exceptions::MissingIDException);
  ASSERT_THROW(property.clearComment(), Exceptions::MissingIDException);
}

TEST_F(PropertyTest, IsOfType) {
  // Setup
  auto coll = db.getCollection("properties");
  auto property = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), 0.0, coll);
  property.link(coll);
  ASSERT_TRUE(property.hasId());
  // Test
  ASSERT_TRUE(property.isOfType<NumberProperty>());
  ASSERT_FALSE(property.isOfType<VectorProperty>());
}

TEST_F(PropertyTest, GetDerived) {
  // Setup
  auto coll = db.getCollection("properties");
  auto numberProperty = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), 5.6, coll);
  Property property(numberProperty.id(), coll);
  ASSERT_TRUE(property.hasId());
  // Test
  auto numprop = property.getDerived<NumberProperty>();
  ASSERT_EQ(numprop.getData(), 5.6);
}

TEST_F(PropertyTest, GetDerivedFails) {
  // Setup
  auto coll = db.getCollection("properties");
  auto test = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), 0.0, coll);
  Property property(test.id());
  ASSERT_THROW(property.getDerived<VectorProperty>(), Exceptions::MissingLinkedCollectionException);
  property.link(coll);
  ASSERT_TRUE(property.hasId());
  ASSERT_THROW(property.getDerived<VectorProperty>(), Exceptions::ObjectTypeMismatchException);
  Property property2;
  property2.link(coll);
  ASSERT_THROW(property2.getDerived<VectorProperty>(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
