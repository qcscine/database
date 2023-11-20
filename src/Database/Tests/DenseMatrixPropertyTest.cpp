/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/DenseMatrixProperty.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class DenseMatrixPropertyTest DenseMatrixPropertyTest.cpp
 * @brief Comprises tests for the class Scine::Database::DenseMatrixProperty.
 * @test
 */
class DenseMatrixPropertyTest : public Test {
 public:
  DenseMatrixPropertyTest() {
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
Credentials DenseMatrixPropertyTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT),
                                                 "unittest_db_DenseMatrixPropertyTest");
Manager DenseMatrixPropertyTest::db;

TEST_F(DenseMatrixPropertyTest, Create1) {
  // Setup
  auto coll = db.getCollection("properties");
  Eigen::MatrixXd data(3, 4);
  // clang-format off
  data << 42.0, 24.0, 40.2, 2.04,
           0.0, 11.1, 22.2, 33.3,
          44.4, 55.5, 66.6, 77.7;
  // clang-format on
  ID s, c;
  DenseMatrixProperty test = DenseMatrixProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), data, s, c, coll);
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
  ASSERT_EQ(data_db(0, 0), 42.0);
  ASSERT_EQ(data_db(2, 0), 44.4);
  ASSERT_EQ(data_db(0, 3), 2.04);
  ASSERT_EQ(data_db(2, 3), 77.7);
}

TEST_F(DenseMatrixPropertyTest, Create2) {
  // Setup
  auto coll = db.getCollection("properties");
  Eigen::MatrixXd data(3, 4);
  // clang-format off
  data << 42.0, 24.0, 40.2, 2.04,
           0.0, 11.1, 22.2, 33.3,
          44.4, 55.5, 66.6, 77.7;
  // clang-format on
  DenseMatrixProperty test =
      DenseMatrixProperty::create("density_matrix", Model("dft", "pbe", "def2-svp", "restricted"), data, coll);
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
  ASSERT_EQ(data_db(0, 0), 42.0);
  ASSERT_EQ(data_db(2, 0), 44.4);
  ASSERT_EQ(data_db(0, 3), 2.04);
  ASSERT_EQ(data_db(2, 3), 77.7);
}

TEST_F(DenseMatrixPropertyTest, Data) {
  // Setup
  auto coll = db.getCollection("properties");
  Eigen::MatrixXd data = Eigen::MatrixXd::Zero(3, 4);
  DenseMatrixProperty test = DenseMatrixProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), data, coll);
  ASSERT_TRUE(test.hasId());

  auto data_db = test.getData();
  ASSERT_EQ(data_db(0, 0), 0.0);
  ASSERT_EQ(data_db(2, 0), 0.0);
  ASSERT_EQ(data_db(0, 3), 0.0);
  ASSERT_EQ(data_db(2, 3), 0.0);
  // clang-format off
  data << 42.0, 24.0, 40.2, 2.04,
           0.0, 11.1, 22.2, 33.3,
          44.4, 55.5, 66.6, 77.7;
  // clang-format on
  test.setData(data);
  data_db = test.getData();
  ASSERT_EQ(data_db(0, 0), 42.0);
  ASSERT_EQ(data_db(2, 0), 44.4);
  ASSERT_EQ(data_db(0, 3), 2.04);
  ASSERT_EQ(data_db(2, 3), 77.7);
}

TEST_F(DenseMatrixPropertyTest, DataFails) {
  // Setup
  auto coll = db.getCollection("properties");
  DenseMatrixProperty test;
  Eigen::MatrixXd data;

  ASSERT_THROW(test.setData(data), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(test.getData(), Exceptions::MissingLinkedCollectionException);
  test.link(coll);
  ASSERT_THROW(test.setData(data), Exceptions::MissingIDException);
  ASSERT_THROW(test.getData(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
