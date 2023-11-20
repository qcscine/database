/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/StringProperty.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class StringPropertyTest StringPropertyTest.cpp
 * @brief Comprises tests for the class Scine::Database::StringProperty.
 * @test
 */
class StringPropertyTest : public Test {
 public:
  StringPropertyTest() {
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
Credentials StringPropertyTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_StringPropertyTest");
Manager StringPropertyTest::db;

TEST_F(StringPropertyTest, CreateDictData) {
  // Setup
  auto coll = db.getCollection("properties");
  ID s, c;
  std::string data = "{'key': 'value'}";
  StringProperty test = StringProperty::create("dictionary", Model("dft", "pbe", "def2-svp"), data, s, c, coll);
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
  ASSERT_EQ(name, "dictionary");
  ASSERT_EQ(calculation, c);
  ASSERT_EQ(structure, s);
  ASSERT_EQ(data_db, data);
}

TEST_F(StringPropertyTest, Create) {
  // Setup
  auto coll = db.getCollection("properties");
  std::string data = "someString";
  StringProperty test = StringProperty::create("simple_string", Model("dft", "pbe", "def2-svp", "restricted"), data, coll);
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
  ASSERT_EQ(name, "simple_string");
  ASSERT_FALSE(test.hasCalculation());
  ASSERT_FALSE(test.hasStructure());
  ASSERT_EQ(data_db, data);
}

TEST_F(StringPropertyTest, changeData) {
  // Setup
  auto coll = db.getCollection("properties");
  std::string data = "{'key': 'value'}";
  StringProperty test =
      StringProperty::create("string_information", Model("dft", "pbe", "def2-svp", "restricted"), data, coll);
  ASSERT_TRUE(test.hasId());

  auto data_db = test.getData();
  ASSERT_EQ(data_db, data);
  test.setData("simpleString");
  data_db = test.getData();
  ASSERT_EQ(data_db, "simpleString");
}

TEST_F(StringPropertyTest, DataFails) {
  // Setup
  auto coll = db.getCollection("properties");
  StringProperty test;

  ASSERT_THROW(test.setData("simpleString"), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(test.getData(), Exceptions::MissingLinkedCollectionException);
  test.link(coll);
  ASSERT_THROW(test.setData("simpleString"), Exceptions::MissingIDException);
  ASSERT_THROW(test.getData(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
