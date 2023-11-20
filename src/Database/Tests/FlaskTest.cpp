/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Flask.h>
#include <Database/Objects/Structure.h>
#include <Utils/Geometry/ElementTypes.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class FlaskTest FlaskTest.cpp
 * @brief Comprises tests for the class Scine::Database::Flask.
 * @test
 */
class FlaskTest : public Test {
 public:
  FlaskTest() {
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
Credentials FlaskTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "utils_test_db_FlaskTest");
Manager FlaskTest::db;

TEST_F(FlaskTest, Create) {
  auto coll = db.getCollection("flasks");
  ID id1, id2, id3;
  Flask flask;
  flask.link(coll);
  ID comp = flask.create({id1}, {id2, id3});
  ASSERT_EQ(flask.id(), comp);
}

TEST_F(FlaskTest, CreateFails) {
  ID id1, id2, id3;
  Flask flask;
  ASSERT_THROW(flask.create({id1}, {id2, id3}), Exceptions::MissingLinkedCollectionException);
}

TEST_F(FlaskTest, CentroidAndCompounds) {
  auto coll = db.getCollection("flasks");
  ID id1, id2, id3;
  Flask flask = Flask::create({id1}, {id2, id3}, coll);
  ASSERT_EQ(flask.getCentroid(), id1);
  ASSERT_EQ(flask.getCompounds()[0], id2);
  ASSERT_EQ(flask.getCompounds()[1], id3);
}

TEST_F(FlaskTest, CentroidFails1) {
  Flask flask;
  ASSERT_THROW(flask.getCentroid(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(FlaskTest, CentroidFails2) {
  auto coll = db.getCollection("flasks");
  Flask flask;
  flask.link(coll);
  ASSERT_THROW(flask.getCentroid(), Exceptions::MissingIDException);
}

TEST_F(FlaskTest, CentroidFails3) {
  auto coll = db.getCollection("flasks");
  Flask flask = Flask::create({}, {}, coll);
  ASSERT_THROW(flask.getCentroid(), Exceptions::MissingIdOrField);
}

TEST_F(FlaskTest, Reaction) {
  auto coll = db.getCollection("flasks");
  ID id1, id2, id3, id4, id5, id6, id7;
  Flask flask = Flask::create({id1}, {id6, id7}, coll);
  // Checks
  ASSERT_FALSE(flask.hasReaction(id1));
  flask.addReaction(id2);
  ASSERT_TRUE(flask.hasReaction(id2));
  ASSERT_EQ(1, flask.hasReactions());
  flask.setReactions({id3, id4, id5});
  ASSERT_EQ(3, flask.hasReactions());
  auto ret = flask.getReactions();
  ASSERT_EQ(ret[0], id3);
  ASSERT_EQ(ret[1], id4);
  ASSERT_EQ(ret[2], id5);
  flask.removeReaction(id4);
  ASSERT_EQ(2, flask.hasReactions());
  ASSERT_FALSE(flask.hasReaction(id4));
  flask.clearReactions();
  ASSERT_EQ(0, flask.hasReactions());
}

TEST_F(FlaskTest, ReactionFails1) {
  Flask flask;
  ID id;
  ASSERT_THROW(flask.hasReaction(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.hasReactions(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.getReactions(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.addReaction(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.setReactions({}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.removeReaction(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.clearReactions(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(FlaskTest, ReactionFails2) {
  auto coll = db.getCollection("flasks");
  Flask flask;
  flask.link(coll);
  ID id;
  ASSERT_THROW(flask.hasReaction(id), Exceptions::MissingIDException);
  ASSERT_THROW(flask.hasReactions(), Exceptions::MissingIDException);
  ASSERT_THROW(flask.getReactions(), Exceptions::MissingIDException);
  ASSERT_THROW(flask.addReaction(id), Exceptions::MissingIDException);
  ASSERT_THROW(flask.setReactions({}), Exceptions::MissingIDException);
  ASSERT_THROW(flask.removeReaction(id), Exceptions::MissingIDException);
  ASSERT_THROW(flask.clearReactions(), Exceptions::MissingIDException);
}

TEST_F(FlaskTest, Structure) {
  auto coll = db.getCollection("flasks");
  ID id1, id2, id3, id4, id5, id6, id7;
  Flask flask = Flask::create({id1}, {id6, id7}, coll);
  // Checks
  ASSERT_TRUE(flask.hasStructure(id1));
  flask.addStructure(id2);
  ASSERT_TRUE(flask.hasStructure(id2));
  ASSERT_EQ(2, flask.hasStructures());
  flask.setStructures({id3, id4, id5});
  ASSERT_EQ(3, flask.hasStructures());
  auto ret = flask.getStructures();
  ASSERT_EQ(ret[0], id3);
  ASSERT_EQ(ret[1], id4);
  ASSERT_EQ(ret[2], id5);
  flask.removeStructure(id4);
  ASSERT_EQ(2, flask.hasStructures());
  ASSERT_FALSE(flask.hasStructure(id4));
  flask.clearStructures();
  ASSERT_EQ(0, flask.hasStructures());
}

TEST_F(FlaskTest, StructureFails1) {
  Flask flask;
  ID id;
  ASSERT_THROW(flask.hasStructure(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.hasStructures(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.getStructures(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.addStructure(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.setStructures({}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.removeStructure(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.clearStructures(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(FlaskTest, StructureFails2) {
  auto coll = db.getCollection("flasks");
  Flask flask;
  flask.link(coll);
  ID id;
  ASSERT_THROW(flask.hasStructure(id), Exceptions::MissingIDException);
  ASSERT_THROW(flask.hasStructures(), Exceptions::MissingIDException);
  ASSERT_THROW(flask.getStructures(), Exceptions::MissingIDException);
  ASSERT_THROW(flask.addStructure(id), Exceptions::MissingIDException);
  ASSERT_THROW(flask.setStructures({}), Exceptions::MissingIDException);
  ASSERT_THROW(flask.removeStructure(id), Exceptions::MissingIDException);
  ASSERT_THROW(flask.clearStructures(), Exceptions::MissingIDException);
}

TEST_F(FlaskTest, Compound) {
  auto coll = db.getCollection("flasks");
  ID id1, id2, id3, id4, id5, id6, id7;
  Flask flask = Flask::create({id1}, {id6, id7}, coll);
  // Checks
  ASSERT_TRUE(flask.hasCompound(id6));
  ASSERT_TRUE(flask.hasCompound(id7));
  ASSERT_EQ(2, flask.hasCompounds());
  flask.setCompounds({id3, id4, id5});
  ASSERT_EQ(3, flask.hasCompounds());
  auto ret = flask.getCompounds();
  ASSERT_EQ(ret[0], id3);
  ASSERT_EQ(ret[1], id4);
  ASSERT_EQ(ret[2], id5);
  flask.clearCompounds();
  ASSERT_EQ(0, flask.hasCompounds());
}

TEST_F(FlaskTest, CompoundFails1) {
  Flask flask;
  ID id;
  ASSERT_THROW(flask.hasCompound(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.hasCompounds(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.getCompounds(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.setCompounds({}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(flask.clearCompounds(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(FlaskTest, CompoundFails2) {
  auto coll = db.getCollection("flasks");
  Flask flask;
  flask.link(coll);
  ID id;
  ASSERT_THROW(flask.hasCompound(id), Exceptions::MissingIDException);
  ASSERT_THROW(flask.hasCompounds(), Exceptions::MissingIDException);
  ASSERT_THROW(flask.getCompounds(), Exceptions::MissingIDException);
  ASSERT_THROW(flask.setCompounds({}), Exceptions::MissingIDException);
  ASSERT_THROW(flask.clearCompounds(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
