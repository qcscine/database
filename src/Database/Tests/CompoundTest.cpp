/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Compound.h>
#include <Database/Objects/Structure.h>
#include <Utils/Geometry/ElementTypes.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class CompoundTest CompoundTest.cpp
 * @brief Comprises tests for the class Scine::Database::Compound.
 * @test
 */
class CompoundTest : public Test {
 public:
  CompoundTest() {
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
Credentials CompoundTest::credentials(TEST_MONGO_DB_IP, 27017, "unittest_db_CompoundTest");
Manager CompoundTest::db;

TEST_F(CompoundTest, Create) {
  auto coll = db.getCollection("compounds");
  ID id;
  Compound compound;
  compound.link(coll);
  ID comp = compound.create({id});
  ASSERT_EQ(compound.id(), comp);
}

TEST_F(CompoundTest, CreateFails) {
  ID id;
  Compound compound;
  ASSERT_THROW(compound.create({id}), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CompoundTest, Centroid) {
  auto coll = db.getCollection("compounds");
  ID id;
  Compound compound = Compound::create({id}, coll);
  ASSERT_EQ(compound.getCentroid(), id);
}

TEST_F(CompoundTest, CentroidFails1) {
  ID id;
  Compound compound;
  ASSERT_THROW(compound.getCentroid(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CompoundTest, CentroidFails2) {
  auto coll = db.getCollection("compounds");
  ID id;
  Compound compound;
  compound.link(coll);
  ASSERT_THROW(compound.getCentroid(), Exceptions::MissingIDException);
}

TEST_F(CompoundTest, CentroidFails3) {
  auto coll = db.getCollection("compounds");
  Compound compound = Compound::create({}, coll);
  ASSERT_THROW(compound.getCentroid(), Exceptions::MissingIdOrField);
}

TEST_F(CompoundTest, Reaction) {
  auto coll = db.getCollection("compounds");
  ID id1, id2, id3, id4, id5;
  Compound compound = Compound::create({id1}, coll);
  // Checks
  ASSERT_FALSE(compound.hasReaction(id1));
  compound.addReaction(id2);
  ASSERT_TRUE(compound.hasReaction(id2));
  ASSERT_EQ(1, compound.hasReactions());
  compound.setReactions({id3, id4, id5});
  ASSERT_EQ(3, compound.hasReactions());
  auto ret = compound.getReactions();
  ASSERT_EQ(ret[0], id3);
  ASSERT_EQ(ret[1], id4);
  ASSERT_EQ(ret[2], id5);
  compound.removeReaction(id4);
  ASSERT_EQ(2, compound.hasReactions());
  ASSERT_FALSE(compound.hasReaction(id4));
  compound.clearReactions();
  ASSERT_EQ(0, compound.hasReactions());
}

TEST_F(CompoundTest, ReactionFails1) {
  Compound compound;
  ID id;
  ASSERT_THROW(compound.hasReaction(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.hasReactions(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.getReactions(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.addReaction(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.setReactions({}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.removeReaction(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.clearReactions(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CompoundTest, ReactionFails2) {
  auto coll = db.getCollection("compounds");
  Compound compound;
  compound.link(coll);
  ID id;
  ASSERT_THROW(compound.hasReaction(id), Exceptions::MissingIDException);
  ASSERT_THROW(compound.hasReactions(), Exceptions::MissingIDException);
  ASSERT_THROW(compound.getReactions(), Exceptions::MissingIDException);
  ASSERT_THROW(compound.addReaction(id), Exceptions::MissingIDException);
  ASSERT_THROW(compound.setReactions({}), Exceptions::MissingIDException);
  ASSERT_THROW(compound.removeReaction(id), Exceptions::MissingIDException);
  ASSERT_THROW(compound.clearReactions(), Exceptions::MissingIDException);
}

TEST_F(CompoundTest, Structure) {
  auto coll = db.getCollection("compounds");
  ID id1, id2, id3, id4, id5;
  Compound compound = Compound::create({id1}, coll);
  // Checks
  ASSERT_TRUE(compound.hasStructure(id1));
  compound.addStructure(id2);
  ASSERT_TRUE(compound.hasStructure(id2));
  ASSERT_EQ(2, compound.hasStructures());
  compound.setStructures({id3, id4, id5});
  ASSERT_EQ(3, compound.hasStructures());
  auto ret = compound.getStructures();
  ASSERT_EQ(ret[0], id3);
  ASSERT_EQ(ret[1], id4);
  ASSERT_EQ(ret[2], id5);
  compound.removeStructure(id4);
  ASSERT_EQ(2, compound.hasStructures());
  ASSERT_FALSE(compound.hasStructure(id4));
  compound.clearStructures();
  ASSERT_EQ(0, compound.hasStructures());
}

TEST_F(CompoundTest, StructureFails1) {
  Compound compound;
  ID id;
  ASSERT_THROW(compound.hasStructure(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.hasStructures(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.getStructures(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.addStructure(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.setStructures({}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.removeStructure(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(compound.clearStructures(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(CompoundTest, StructureFails2) {
  auto coll = db.getCollection("compounds");
  Compound compound;
  compound.link(coll);
  ID id;
  ASSERT_THROW(compound.hasStructure(id), Exceptions::MissingIDException);
  ASSERT_THROW(compound.hasStructures(), Exceptions::MissingIDException);
  ASSERT_THROW(compound.getStructures(), Exceptions::MissingIDException);
  ASSERT_THROW(compound.addStructure(id), Exceptions::MissingIDException);
  ASSERT_THROW(compound.setStructures({}), Exceptions::MissingIDException);
  ASSERT_THROW(compound.removeStructure(id), Exceptions::MissingIDException);
  ASSERT_THROW(compound.clearStructures(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
