/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/NumberProperty.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class NumberPropertyTest NumberPropertyTest.cpp
 * @brief Comprises tests for the class Scine::Database::NumberProperty.
 * @test
 */
class NumberPropertyTest : public Test {
 public:
  NumberPropertyTest() {
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
Credentials NumberPropertyTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_NumberPropertyTest");
Manager NumberPropertyTest::db;

TEST_F(NumberPropertyTest, Create1) {
  // Setup
  auto coll = db.getCollection("properties");
  ID s, c;
  NumberProperty test = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), 7.0, s, c, coll);
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
  ASSERT_EQ(data_db, 7.0);
}

TEST_F(NumberPropertyTest, Create2) {
  // Setup
  auto coll = db.getCollection("properties");
  NumberProperty test = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp", "restricted"), 7.0, coll);
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
  ASSERT_EQ(data_db, 7.0);
}

TEST_F(NumberPropertyTest, Data) {
  // Setup
  auto coll = db.getCollection("properties");
  NumberProperty test = NumberProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), 0.0, coll);
  ASSERT_TRUE(test.hasId());

  auto data_db = test.getData();
  ASSERT_EQ(data_db, 0.0);
  test.setData(7.0);
  data_db = test.getData();
  ASSERT_EQ(data_db, 7.0);
}

TEST_F(NumberPropertyTest, DataFails) {
  // Setup
  auto coll = db.getCollection("properties");
  NumberProperty test;

  ASSERT_THROW(test.setData(7.0), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(test.getData(), Exceptions::MissingLinkedCollectionException);
  test.link(coll);
  ASSERT_THROW(test.setData(7.0), Exceptions::MissingIDException);
  ASSERT_THROW(test.getData(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
