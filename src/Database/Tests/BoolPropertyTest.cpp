/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/BoolProperty.h>
#include <Database/Objects/Model.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class BoolPropertyTest BoolPropertyTest.cpp
 * @brief Comprises tests for the class Scine::Database::BoolProperty.
 * @test
 */
class BoolPropertyTest : public Test {
 public:
  BoolPropertyTest() {
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
Credentials BoolPropertyTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_BoolPropertyTest");
Manager BoolPropertyTest::db;

TEST_F(BoolPropertyTest, Create1) {
  // Setup
  auto coll = db.getCollection("properties");
  ID s, c;
  BoolProperty test = BoolProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), true, s, c, coll);
  ASSERT_TRUE(test.hasId());

  // Check Fields
  auto model = test.getModel();
  auto comment = test.getComment();
  auto name = test.getPropertyName();
  auto calculation = test.getCalculation();
  auto structure = test.getStructure();
  auto data_db = test.getData();

  ASSERT_EQ(model.method, "pbe");
  ASSERT_EQ(model.basisSet, "def2-svp");
  ASSERT_EQ(model.spinMode, "any");
  ASSERT_EQ(comment, "");
  ASSERT_EQ(name, "density_matrix");
  ASSERT_EQ(calculation, c);
  ASSERT_EQ(structure, s);
  ASSERT_EQ(data_db, true);
}

TEST_F(BoolPropertyTest, Create2) {
  // Setup
  auto coll = db.getCollection("properties");
  BoolProperty test = BoolProperty::create("density_matrix", Model("dft", "pbe", "def2-svp", "restricted"), false, coll);
  ASSERT_TRUE(test.hasId());

  // Check Fields
  auto model = test.getModel();
  auto comment = test.getComment();
  auto name = test.getPropertyName();
  auto data_db = test.getData();

  ASSERT_EQ(model.method, "pbe");
  ASSERT_EQ(model.basisSet, "def2-svp");
  ASSERT_EQ(model.spinMode, "restricted");
  ASSERT_EQ(comment, "");
  ASSERT_EQ(name, "density_matrix");
  ASSERT_FALSE(test.hasCalculation());
  ASSERT_FALSE(test.hasStructure());
  ASSERT_EQ(data_db, false);
}

TEST_F(BoolPropertyTest, Data) {
  // Setup
  auto coll = db.getCollection("properties");
  BoolProperty test = BoolProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), true, coll);
  ASSERT_TRUE(test.hasId());

  auto data_db = test.getData();
  ASSERT_EQ(data_db, true);
  test.setData(false);
  data_db = test.getData();
  ASSERT_EQ(data_db, false);
}

TEST_F(BoolPropertyTest, DataFails) {
  // Setup
  auto coll = db.getCollection("properties");
  BoolProperty test;

  ASSERT_THROW(test.setData(true), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(test.getData(), Exceptions::MissingLinkedCollectionException);
  test.link(coll);
  ASSERT_THROW(test.setData(true), Exceptions::MissingIDException);
  ASSERT_THROW(test.getData(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
