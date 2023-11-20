/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/SparseMatrixProperty.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class SparseMatrixPropertyTest SparseMatrixPropertyTest.cpp
 * @brief Comprises tests for the class Scine::Database::SparseMatrixProperty.
 * @test
 */
class SparseMatrixPropertyTest : public Test {
 public:
  SparseMatrixPropertyTest() {
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
Credentials SparseMatrixPropertyTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT),
                                                  "unittest_db_SparseMatrixPropertyTest");
Manager SparseMatrixPropertyTest::db;

TEST_F(SparseMatrixPropertyTest, Create1) {
  // Setup
  auto coll = db.getCollection("properties");
  Eigen::SparseMatrix<double> data(3, 4);
  data.insert(0, 0) = 42.0;
  data.insert(0, 2) = 44.4;
  data.insert(2, 0) = 40.2;
  data.insert(2, 3) = 77.7;
  ID s, c;
  SparseMatrixProperty test =
      SparseMatrixProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), data, s, c, coll);
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
  ASSERT_EQ(data_db.coeff(0, 0), 42.0);
  ASSERT_EQ(data_db.coeff(0, 2), 44.4);
  ASSERT_EQ(data_db.coeff(2, 0), 40.2);
  ASSERT_EQ(data_db.coeff(2, 3), 77.7);
}

TEST_F(SparseMatrixPropertyTest, Create2) {
  // Setup
  auto coll = db.getCollection("properties");
  Eigen::SparseMatrix<double> data(3, 4);
  data.insert(0, 0) = 42.0;
  data.insert(2, 0) = 44.4;
  data.insert(0, 2) = 40.2;
  data.insert(2, 3) = 77.7;
  SparseMatrixProperty test =
      SparseMatrixProperty::create("density_matrix", Model("dft", "pbe", "def2-svp", "restricted"), data, coll);
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
  ASSERT_EQ(data_db.coeff(0, 0), 42.0);
  ASSERT_EQ(data_db.coeff(2, 0), 44.4);
  ASSERT_EQ(data_db.coeff(0, 2), 40.2);
  ASSERT_EQ(data_db.coeff(2, 3), 77.7);
}

TEST_F(SparseMatrixPropertyTest, Data) {
  // Setup
  auto coll = db.getCollection("properties");
  Eigen::SparseMatrix<double> data(3, 4);
  SparseMatrixProperty test = SparseMatrixProperty::create("density_matrix", Model("dft", "pbe", "def2-svp"), data, coll);
  ASSERT_TRUE(test.hasId());

  auto data_db = test.getData();
  ASSERT_EQ(data_db.coeff(0, 0), 0.0);
  ASSERT_EQ(data_db.coeff(2, 0), 0.0);
  ASSERT_EQ(data_db.coeff(0, 2), 0.0);
  ASSERT_EQ(data_db.coeff(2, 3), 0.0);
  data.insert(0, 0) = 42.0;
  data.insert(0, 2) = 44.4;
  data.insert(2, 0) = 40.2;
  data.insert(2, 3) = 77.7;
  test.setData(data);
  data_db = test.getData();
  ASSERT_EQ(data_db.coeff(0, 0), 42.0);
  ASSERT_EQ(data_db.coeff(0, 2), 44.4);
  ASSERT_EQ(data_db.coeff(2, 0), 40.2);
  ASSERT_EQ(data_db.coeff(2, 3), 77.7);
}

TEST_F(SparseMatrixPropertyTest, DataFails) {
  // Setup
  auto coll = db.getCollection("properties");
  SparseMatrixProperty test;
  Eigen::SparseMatrix<double> data;

  ASSERT_THROW(test.setData(data), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(test.getData(), Exceptions::MissingLinkedCollectionException);
  test.link(coll);
  ASSERT_THROW(test.setData(data), Exceptions::MissingIDException);
  ASSERT_THROW(test.getData(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
