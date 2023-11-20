/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Compound.h>
#include <Database/Objects/Structure.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class CollectionTest CollectionTest.cpp
 * @brief Comprises tests for the class Scine::Database::Collection.
 * @test
 */
struct CollectionTest : public Test {
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

Credentials CollectionTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_CollectionTest");
Manager CollectionTest::db;

TEST_F(CollectionTest, QueryID) {
  // Setup
  auto coll = db.getCollection("compounds");
  ID id1;
  Compound comp = Compound::create({id1}, coll);

  // Check
  ASSERT_TRUE(coll->has(comp.id()));
}

TEST_F(CollectionTest, ReturnObjectByID) {
  // Setup
  auto coll = db.getCollection("compounds");
  ID id1;
  Compound comp = Compound::create({id1}, coll);

  // Get
  auto result = coll->get<Compound>(comp.id());
  ASSERT_EQ(result.id(), comp.id());
}

TEST_F(CollectionTest, ReturnObjectByIDFails) {
  auto coll = db.getCollection("compounds");
  ID id1;
  ASSERT_THROW(coll->get<Compound>(id1), Exceptions::IDNotFoundException);
}

TEST_F(CollectionTest, QueryByJSON) {
  // Make sure the DB is clean in order to allow for accurate counts.
  db.wipe();
  db.init();
  // Setup
  auto coll = db.getCollection("compounds");
  ID id1, id2, id3;
  Compound comp1 = Compound::create({id1}, coll);
  Compound comp2 = Compound::create({id1}, coll);
  Compound comp3 = Compound::create({id3}, coll);

  std::string query = R"({ "structures" : { "$eq" : { "$oid" : ")" + id1.string() + "\" } } }";
  auto vec1 = coll->query<Compound>(query);
  auto vec2 = coll->query<Structure>(query);

  // Check
  ASSERT_EQ(vec1.size(), 2);
  ASSERT_EQ(vec1[0].id(), comp1.id());
  ASSERT_EQ(vec1[1].id(), comp2.id());
  ASSERT_EQ(vec2.size(), 0);
}

TEST_F(CollectionTest, GetOne) {
  // Make sure the DB is clean in order to allow for accurate counts.
  db.wipe();
  db.init();
  // Setup
  auto coll = db.getCollection("compounds");
  ID id1, id2, id3;
  Compound comp1 = Compound::create({id1}, coll);
  Compound comp2 = Compound::create({id1}, coll);
  Compound comp3 = Compound::create({id3}, coll);
  // Create a compound that could be found but should not to catch false
  // positives

  const std::string query1 = R"({ "structures" : { "$eq" : { "$oid" : ")" + id1.string() + "\" } } }";
  const std::string query2 = R"({ "structures" : { "$eq" : { "$oid" : ")" + id2.string() + "\" } } }";
  auto r1 = coll->getOne<Compound>(query1);
  auto r2 = coll->getOne<Compound>(query2);

  // Check
  ASSERT_TRUE(r1.hasId());
  ASSERT_TRUE(r1.id() == comp1.id());
  ASSERT_FALSE(r2.hasId());
  ASSERT_THROW(coll->getOne<Structure>(query1), Exceptions::ObjectTypeMismatchException);
}

TEST_F(CollectionTest, GetOneWithSort) {
  // Make sure the DB is clean in order to allow for accurate counts.
  db.wipe();
  db.init();
  // Setup
  auto coll = db.getCollection("compounds");
  ID id1, id2;
  Compound comp1 = Compound::create({id1}, coll);
  Compound comp2 = Compound::create({id2, id1}, coll);

  const std::string query = R"({ "structures" : { "$all" : [{ "$oid" : ")" + id1.string() + "\" }] } }";
  const std::string sort1 = R"({"_id": 1})";
  const std::string sort2 = R"({"_id": -1})";
  auto r1 = coll->getOne<Compound>(query);
  auto r2 = coll->getOne<Compound>(query, sort1);
  auto r3 = coll->getOne<Compound>(query, sort2);

  ASSERT_TRUE(r1.hasId());
  ASSERT_TRUE(r2.hasId());
  ASSERT_TRUE(r3.hasId());

  ASSERT_TRUE(r1.id() == comp1.id());
  ASSERT_TRUE(r2.id() == comp1.id());
  ASSERT_TRUE(r3.id() == comp2.id());

  ASSERT_TRUE(r1.getStructures()[0] == id1);
  ASSERT_TRUE(r2.getStructures()[0] == id1);
  ASSERT_TRUE(r3.getStructures()[0] == id2);

  ASSERT_THROW(coll->getOne<Structure>(query, sort1), Exceptions::ObjectTypeMismatchException);
}

TEST_F(CollectionTest, GetOneAndModify) {
  // Make sure the DB is clean in order to allow for accurate counts.
  db.wipe();
  db.init();
  // Setup
  auto coll = db.getCollection("compounds");
  ID id1, id2, id3, id4;
  Compound comp1 = Compound::create({id1}, coll);
  Compound comp2 = Compound::create({id1}, coll);
  Compound comp3 = Compound::create({id3}, coll);
  // Create a compound that could be found but should not to catch false
  // positives

  const std::string query1 = R"({ "structures" : { "$eq" : { "$oid" : ")" + id1.string() + "\" } } }";
  const std::string query2 = R"({ "structures" : { "$eq" : { "$oid" : ")" + id2.string() + "\" } } }";
  const std::string update = R"({ "$set" : { "reactions" : [{ "$oid" : ")" + id4.string() + "\" }] } }";
  auto r1 = coll->getAndUpdateOne<Compound>(query1, update);
  auto r2 = coll->getAndUpdateOne<Compound>(query2, update);

  // Check
  ASSERT_TRUE(r1.hasId());
  ASSERT_TRUE(r1.id() == comp1.id());
  ASSERT_TRUE(r1.getReactions()[0] == id4);
  ASSERT_FALSE(r2.hasId());
  ASSERT_THROW(coll->getAndUpdateOne<Structure>(query1, update), Exceptions::ObjectTypeMismatchException);
}

TEST_F(CollectionTest, GetOneAndModifyWithSort) {
  // Make sure the DB is clean in order to allow for accurate counts.
  db.wipe();
  db.init();
  // Setup
  auto coll = db.getCollection("compounds");
  ID id1, id2, id3;
  Compound comp1 = Compound::create({id1}, coll);
  Compound comp2 = Compound::create({id2, id1}, coll);
  // Create a compound that could be found but should not to catch false
  // positives

  const std::string query = R"({ "structures" : { "$all" : [{ "$oid" : ")" + id1.string() + "\" }] } }";
  const std::string update = R"({ "$set" : { "reactions" : [{ "$oid" : ")" + id3.string() + "\" }] } }";
  const std::string sort = R"({"_id": -1})";
  auto r1 = coll->getAndUpdateOne<Compound>(query, update, sort);

  // Check
  ASSERT_TRUE(r1.hasId());
  ASSERT_TRUE(r1.id() == comp2.id());
  ASSERT_TRUE(r1.getReactions()[0] == id3);

  ASSERT_THROW(coll->getAndUpdateOne<Structure>(query, update, sort), Exceptions::ObjectTypeMismatchException);
}

TEST_F(CollectionTest, RandomSelect) {
  // Make sure the DB is clean in order to allow for accurate counts.
  db.wipe();
  db.init();
  // Setup
  auto coll = db.getCollection("compounds");
  ID id1, id2, id3;
  Compound comp1 = Compound::create({id1}, coll);
  Compound comp2 = Compound::create({id2}, coll);
  Compound comp3 = Compound::create({id3}, coll);

  // Check correct number
  std::int32_t nSamples = 2;
  std::vector<Compound> vec1 = coll->randomSelect<Compound>(nSamples);
  ASSERT_EQ(vec1.size(), nSamples);

  // Check if higher number is possible
  nSamples = 10;
  std::vector<Compound> vec2 = coll->randomSelect<Compound>(nSamples);
  ASSERT_NE(vec2.size(), nSamples);
  ASSERT_EQ(vec2.size(), coll->count("{}"));
}

TEST_F(CollectionTest, CountByJSON) {
  // Make sure the DB is clean in order to allow for accurate counts.
  db.wipe();
  db.init();
  // Setup
  auto coll = db.getCollection("compounds");
  ID id1, id2;
  Compound comp1 = Compound::create({id1}, coll);
  Compound comp2 = Compound::create({id1}, coll);
  Compound comp3 = Compound::create({id2}, coll);

  std::string query = R"({ "structures" : { "$eq" : { "$oid" : ")" + id1.string() + "\" } } }";
  auto result = coll->count(query);

  // Check
  ASSERT_EQ(result, 2);
}

TEST_F(CollectionTest, TestLoop) {
  // Make sure the DB is clean in order to allow for accurate counts.
  db.wipe();
  db.init();
  // Setup
  auto coll = db.getCollection("compounds");
  ID id1, id2;
  Compound comp1 = Compound::create({id1}, coll);
  Compound comp2 = Compound::create({id1}, coll);
  Compound comp3 = Compound::create({id2}, coll);

  std::string query = R"({ "structures" : { "$eq" : { "$oid" : ")" + id1.string() + "\" } } }";

  int count = 0;
  for (auto iter = coll->iteratorQuery<Compound>(query); !iter.done(); iter++) {
    count++;
    auto comp = *iter;
    comp.link(coll);
    ASSERT_TRUE(comp.hasStructure(id1));
  }

  // Check
  ASSERT_EQ(count, 2);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
