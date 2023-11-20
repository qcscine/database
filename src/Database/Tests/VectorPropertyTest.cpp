/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/VectorProperty.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class VectorPropertyTest VectorPropertyTest.cpp
 * @brief Comprises tests for the class Scine::Database::VectorProperty.
 * @test
 */
class VectorPropertyTest : public Test {
 public:
  VectorPropertyTest() {
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
Credentials VectorPropertyTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_VectorPropertyTest");
Manager VectorPropertyTest::db;

TEST_F(VectorPropertyTest, Create1) {
  // Setup
  auto coll = db.getCollection("properties");
  Eigen::VectorXd data(4);
  data << 42.0, 24.0, 40.2, 2.04;
  ID s, c;
  VectorProperty test = VectorProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), data, s, c, coll);
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
  ASSERT_EQ(data_db[0], 42.0);
  ASSERT_EQ(data_db[1], 24.0);
  ASSERT_EQ(data_db[2], 40.2);
  ASSERT_EQ(data_db[3], 2.04);
}

TEST_F(VectorPropertyTest, Create2) {
  // Setup
  auto coll = db.getCollection("properties");
  Eigen::VectorXd data(4);
  data << 42.0, 24.0, 40.2, 2.04;
  VectorProperty test = VectorProperty::create("density_matrix", Model("dft", "pbe", "def2-svp", "restricted"), data, coll);
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
  ASSERT_EQ(data_db[0], 42.0);
  ASSERT_EQ(data_db[1], 24.0);
  ASSERT_EQ(data_db[2], 40.2);
  ASSERT_EQ(data_db[3], 2.04);
}

TEST_F(VectorPropertyTest, Data) {
  // Setup
  auto coll = db.getCollection("properties");
  Eigen::VectorXd data = Eigen::VectorXd::Zero(4);
  VectorProperty test = VectorProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), data, coll);
  ASSERT_TRUE(test.hasId());

  auto data_db = test.getData();
  ASSERT_EQ(data_db[0], 0.0);
  ASSERT_EQ(data_db[1], 0.0);
  ASSERT_EQ(data_db[2], 0.0);
  ASSERT_EQ(data_db[3], 0.0);
  // clang-format off
  data << 42.0, 24.0, 40.2, 2.04;
  // clang-format on
  test.setData(data);
  data_db = test.getData();
  ASSERT_EQ(data_db[0], 42.0);
  ASSERT_EQ(data_db[1], 24.0);
  ASSERT_EQ(data_db[2], 40.2);
  ASSERT_EQ(data_db[3], 2.04);
}

TEST_F(VectorPropertyTest, DataFails) {
  // Setup
  auto coll = db.getCollection("properties");
  VectorProperty test;
  Eigen::VectorXd data;

  ASSERT_THROW(test.setData(data), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(test.getData(), Exceptions::MissingLinkedCollectionException);
  test.link(coll);
  ASSERT_THROW(test.setData(data), Exceptions::MissingIDException);
  ASSERT_THROW(test.getData(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
