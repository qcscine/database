/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Object.h>
#include <Utils/UniversalSettings/ValueCollection.h>
#include <gmock/gmock.h>
/* External Includes */
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/collection.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class ObjectTest ObjectTest.cpp
 * @brief Comprises tests for the class Scine::Database::Object.
 * @test
 */
class ObjectTest : public Test {
 public:
  ObjectTest() {
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
Credentials ObjectTest::credentials(TEST_MONGO_DB_IP, 27017, "unittest_db_ObjectTest");
Manager ObjectTest::db;

/// @brief A MockObject used to test public functions of the Object interface.
class MockObject : public Object {
 public:
  /// @brief The name of this derived database object.
  static constexpr const char* objecttype = "MockObject";

  // Inherit constructors
  using Object::Object;

  static MockObject create(CollectionPtr collection, int i) {
    auto now = bsoncxx::types::b_date(std::chrono::system_clock::now());
    // clang-format off
    auto mock = document{} << "_created" << now
                           << "_lastmodified" << now
                           << "analysis_disabled" << false
                           << "exploration_disabled" << false
                           << "mock" << i
                           << finalize;
    // clang-format on
    if (!collection) {
      throw Exceptions::MissingCollectionException();
    }

    auto result = collection->mongocxx().insert_one(mock.view());
    return MockObject{result->inserted_id().get_oid().value, std::move(collection)};
  }
};

TEST(IDTest, StringConstructor) {
  // Setup
  std::string str = "5be431a11afe220ada32c1d4";
  ID id(str);

  ASSERT_EQ(str, id.string());
}

TEST_F(ObjectTest, Linking) {
  auto coll = db.getCollection("properties");
  MockObject mock;
  EXPECT_FALSE(mock.hasLink());
  mock.link(coll);
  EXPECT_EQ(mock.collection(), coll);
  EXPECT_TRUE(mock.hasLink());
  mock.detach();
  EXPECT_FALSE(mock.hasLink());
}

TEST_F(ObjectTest, Creation) {
  auto coll = db.getCollection("properties");
  auto mock = MockObject::create(coll, 5);
  EXPECT_TRUE(mock.hasId());
  EXPECT_TRUE(mock.exists());
  mock.wipe();
  EXPECT_FALSE(mock.hasId());
}

TEST_F(ObjectTest, Copy) {
  auto coll = db.getCollection("properties");
  MockObject mock;
  MockObject copy1, copy3;

  copy1 = mock;
  EXPECT_FALSE(copy1.hasId());
  EXPECT_FALSE(copy1.hasLink());
  MockObject copy2(mock);
  EXPECT_FALSE(copy2.hasId());
  EXPECT_FALSE(copy2.hasLink());

  mock = MockObject::create(coll, 5);

  copy3 = mock;
  EXPECT_TRUE(copy3.hasId());
  EXPECT_TRUE(copy3.hasLink());
  MockObject copy4(mock);
  EXPECT_TRUE(copy4.hasId());
  EXPECT_TRUE(copy4.hasLink());
}

TEST_F(ObjectTest, Delete) {
  std::shared_ptr<Collection> coll = db.getCollection("properties");
  MockObject mock = MockObject::create(coll, 5);
  ID id = mock.id();
  mock.link(coll);
  ASSERT_TRUE(coll->has(id));
  mock.wipe();
  ASSERT_FALSE(coll->has(id));
  ASSERT_FALSE(mock.hasId());
}

TEST_F(ObjectTest, CreationFails) {
  ASSERT_THROW(MockObject::create(nullptr, 5), Exceptions::MissingCollectionException);
}

TEST_F(ObjectTest, CollectionFails) {
  MockObject mock;
  ASSERT_THROW(mock.collection(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ObjectTest, IDFails) {
  MockObject mock;
  ASSERT_THROW(mock.id(), Exceptions::MissingIDException);
}

TEST_F(ObjectTest, CheckExistenceFails_Link) {
  // Setup
  MockObject mock;
  ASSERT_THROW(mock.exists(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ObjectTest, CheckExistenceFails_ID) {
  // Setup
  auto coll = db.getCollection("properties");
  MockObject mock;
  mock.link(coll);
  ASSERT_THROW(mock.exists(), Exceptions::MissingIDException);
}

TEST_F(ObjectTest, JSON) {
  auto coll = db.getCollection("properties");
  MockObject mock = MockObject::create(coll, 5);
  ASSERT_NE(mock.json(), "");
}

TEST_F(ObjectTest, Print) {
  auto coll = db.getCollection("properties");
  MockObject mock = MockObject::create(coll, 5);
  testing::internal::CaptureStdout();
  mock.print();
  std::string output = testing::internal::GetCapturedStdout();
  ASSERT_NE(output, "");
}

TEST_F(ObjectTest, Dates) {
  auto coll = db.getCollection("properties");
  MockObject mock1 = MockObject::create(coll, 5);
  sleep(1);
  MockObject mock2 = MockObject::create(coll, 5);
  mock1.created();
  mock1.lastModified();
  ASSERT_TRUE(mock1.hasCreatedTimestamp());
  ASSERT_TRUE(mock1.hasLastModifiedTimestamp());
  ASSERT_TRUE(mock1.olderThan(mock2, false));
  ASSERT_TRUE(mock1.olderThan(mock2, true));
  mock1.touch();
  ASSERT_TRUE(mock2.olderThan(mock1, true));
}

TEST_F(ObjectTest, DateFails1) {
  MockObject mock;
  ASSERT_THROW(mock.created(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(mock.hasCreatedTimestamp(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(mock.touch(), Exceptions::MissingLinkedCollectionException);
  auto coll = db.getCollection("properties");
  mock.link(coll);
  ASSERT_THROW(mock.created(), Exceptions::MissingIDException);
  ASSERT_THROW(mock.hasCreatedTimestamp(), Exceptions::MissingIDException);
  ASSERT_THROW(mock.touch(), Exceptions::MissingIDException);
}

TEST_F(ObjectTest, DateFails2) {
  MockObject mock;
  ASSERT_THROW(mock.lastModified(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(mock.hasLastModifiedTimestamp(), Exceptions::MissingLinkedCollectionException);
  auto coll = db.getCollection("properties");
  mock.link(coll);
  ASSERT_THROW(mock.lastModified(), Exceptions::MissingIDException);
  ASSERT_THROW(mock.hasLastModifiedTimestamp(), Exceptions::MissingIDException);
}

TEST_F(ObjectTest, Disabling) {
  auto coll = db.getCollection("properties");
  MockObject mock;
  // Test exceptions
  ASSERT_THROW(mock.explore(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(mock.analyze(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(mock.disable_analysis(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(mock.disable_exploration(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(mock.enable_analysis(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(mock.enable_exploration(), Exceptions::MissingLinkedCollectionException);
  // Link and create
  mock = MockObject::create(coll, 5);
  // Test functionality
  EXPECT_TRUE(mock.analyze());
  EXPECT_TRUE(mock.explore());
  mock.disable_analysis();
  EXPECT_FALSE(mock.analyze());
  mock.disable_exploration();
  EXPECT_FALSE(mock.explore());
  mock.enable_analysis();
  EXPECT_TRUE(mock.analyze());
  mock.enable_exploration();
  EXPECT_TRUE(mock.explore());
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
