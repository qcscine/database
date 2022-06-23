/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Reaction.h>
#include <Database/Objects/ReactionSide.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class ReactionTest ReactionTest.cpp
 * @brief Comprises tests for the class Scine::Database::Reaction.
 * @test
 */
class ReactionTest : public Test {
 public:
  ReactionTest() {
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
Credentials ReactionTest::credentials(TEST_MONGO_DB_IP, 27017, "unittest_db_ReactionTest");
Manager ReactionTest::db;

TEST_F(ReactionTest, Create) {
  auto coll = db.getCollection("reactions");
  ID id1, id2;
  Reaction reaction = Reaction::create({id1}, {id2}, coll);
  ASSERT_EQ(std::get<0>(reaction.getReactants(SIDE::BOTH))[0], id1);
  ASSERT_EQ(std::get<1>(reaction.getReactants(SIDE::BOTH))[0], id2);
}

TEST_F(ReactionTest, ReactantLHS) {
  // Basic setup
  auto coll = db.getCollection("reactions");
  ID id1, id2, id3, id4, id5, id6;
  Reaction reaction = Reaction::create({id1}, {id2}, coll);
  // Checks
  ASSERT_EQ(reaction.hasReactant(id1), SIDE::LHS);
  ASSERT_EQ(reaction.hasReactant(id2), SIDE::RHS);
  ASSERT_EQ(1, std::get<0>(reaction.hasReactants()));
  ASSERT_EQ(1, std::get<1>(reaction.hasReactants()));
  reaction.addReactant(id3, SIDE::LHS);
  ASSERT_EQ(reaction.hasReactant(id3), SIDE::LHS);
  ASSERT_EQ(2, std::get<0>(reaction.hasReactants()));
  ASSERT_EQ(1, std::get<1>(reaction.hasReactants()));
  auto reactants = reaction.getReactants(SIDE::LHS);
  ASSERT_EQ(std::get<0>(reactants)[0], id1);
  ASSERT_EQ(std::get<0>(reactants)[1], id3);
  ASSERT_EQ(std::get<1>(reactants).size(), 0);
  reaction.setReactants({id4, id5, id6}, SIDE::LHS);
  ASSERT_EQ(3, std::get<0>(reaction.hasReactants()));
  reaction.removeReactant(id5, SIDE::LHS);
  ASSERT_EQ(2, std::get<0>(reaction.hasReactants()));
  reaction.clearReactants(SIDE::LHS);
  ASSERT_EQ(0, std::get<0>(reaction.hasReactants()));
}

TEST_F(ReactionTest, ReactantRHS) {
  // Basic setup
  auto coll = db.getCollection("reactions");
  ID id1, id2, id3, id4, id5, id6;
  Reaction reaction = Reaction::create({id1}, {id2}, coll);
  // Checks
  ASSERT_EQ(reaction.hasReactant(id1), SIDE::LHS);
  ASSERT_EQ(reaction.hasReactant(id2), SIDE::RHS);
  ASSERT_EQ(1, std::get<0>(reaction.hasReactants()));
  ASSERT_EQ(1, std::get<1>(reaction.hasReactants()));
  reaction.addReactant(id3, SIDE::RHS);
  ASSERT_EQ(reaction.hasReactant(id3), SIDE::RHS);
  ASSERT_EQ(1, std::get<0>(reaction.hasReactants()));
  ASSERT_EQ(2, std::get<1>(reaction.hasReactants()));
  auto reactants = reaction.getReactants(SIDE::RHS);
  ASSERT_EQ(std::get<1>(reactants)[0], id2);
  ASSERT_EQ(std::get<1>(reactants)[1], id3);
  ASSERT_EQ(std::get<0>(reactants).size(), 0);
  reaction.setReactants({id4, id5, id6}, SIDE::RHS);
  ASSERT_EQ(3, std::get<1>(reaction.hasReactants()));
  reaction.removeReactant(id5, SIDE::RHS);
  ASSERT_EQ(2, std::get<1>(reaction.hasReactants()));
  reaction.clearReactants(SIDE::RHS);
  ASSERT_EQ(0, std::get<1>(reaction.hasReactants()));
}

TEST_F(ReactionTest, ReactantBOTH) {
  // Basic setup
  auto coll = db.getCollection("reactions");
  ID id1, id2, id3, id4, id5, id6;
  Reaction reaction = Reaction::create({id1}, {id2}, coll);
  // Checks
  ASSERT_EQ(reaction.hasReactant(id1), SIDE::LHS);
  ASSERT_EQ(reaction.hasReactant(id2), SIDE::RHS);
  ASSERT_EQ(1, std::get<0>(reaction.hasReactants()));
  ASSERT_EQ(1, std::get<1>(reaction.hasReactants()));
  reaction.addReactant(id3, SIDE::BOTH);
  ASSERT_EQ(reaction.hasReactant(id3), SIDE::BOTH);
  ASSERT_EQ(2, std::get<0>(reaction.hasReactants()));
  ASSERT_EQ(2, std::get<1>(reaction.hasReactants()));
  auto reactants = reaction.getReactants(SIDE::BOTH);
  ASSERT_EQ(std::get<0>(reactants)[0], id1);
  ASSERT_EQ(std::get<0>(reactants)[1], id3);
  ASSERT_EQ(std::get<1>(reactants)[0], id2);
  ASSERT_EQ(std::get<1>(reactants)[1], id3);
  reaction.setReactants({id4, id5, id6}, SIDE::BOTH);
  ASSERT_EQ(3, std::get<0>(reaction.hasReactants()));
  ASSERT_EQ(3, std::get<1>(reaction.hasReactants()));
  reaction.removeReactant(id5, SIDE::BOTH);
  ASSERT_EQ(2, std::get<0>(reaction.hasReactants()));
  ASSERT_EQ(2, std::get<1>(reaction.hasReactants()));
  reaction.clearReactants(SIDE::BOTH);
  ASSERT_EQ(0, std::get<0>(reaction.hasReactants()));
  ASSERT_EQ(0, std::get<1>(reaction.hasReactants()));
}

TEST_F(ReactionTest, ReactantFails1) {
  Reaction reaction;
  ID id;
  ASSERT_THROW(reaction.hasReactant(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.hasReactants(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.getReactants(SIDE::BOTH), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.addReactant(id, SIDE::BOTH), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.setReactants({}, SIDE::BOTH), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.removeReactant(id, SIDE::BOTH), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.clearReactants(SIDE::BOTH), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ReactionTest, ReactantFails2) {
  auto coll = db.getCollection("reactions");
  Reaction reaction;
  reaction.link(coll);
  ID id;
  ASSERT_THROW(reaction.hasReactant(id), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.hasReactants(), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.getReactants(SIDE::BOTH), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.addReactant(id, SIDE::BOTH), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.setReactants({}, SIDE::BOTH), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.removeReactant(id, SIDE::BOTH), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.clearReactants(SIDE::BOTH), Exceptions::MissingIDException);
}

TEST_F(ReactionTest, ElementaryStep) {
  auto coll = db.getCollection("reactions");
  ID id1, id2, id3, id4, id5;
  Reaction reaction = Reaction::create({id1}, {id2}, coll);
  // Checks
  ASSERT_FALSE(reaction.hasElementaryStep(id1));
  reaction.addElementaryStep(id2);
  ASSERT_TRUE(reaction.hasElementaryStep(id2));
  ASSERT_EQ(1, reaction.hasElementarySteps());
  reaction.setElementarySteps({id3, id4, id5});
  ASSERT_EQ(3, reaction.hasElementarySteps());
  auto ret = reaction.getElementarySteps();
  ASSERT_EQ(ret[0], id3);
  ASSERT_EQ(ret[1], id4);
  ASSERT_EQ(ret[2], id5);
  reaction.removeElementaryStep(id4);
  ASSERT_EQ(2, reaction.hasElementarySteps());
  ASSERT_FALSE(reaction.hasElementaryStep(id4));
  reaction.clearElementarySteps();
  ASSERT_EQ(0, reaction.hasElementarySteps());
}

TEST_F(ReactionTest, ElementaryStepFails1) {
  Reaction reaction;
  ID id;
  ASSERT_THROW(reaction.hasElementaryStep(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.hasElementarySteps(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.getElementarySteps(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.addElementaryStep(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.setElementarySteps({}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.removeElementaryStep(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(reaction.clearElementarySteps(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ReactionTest, ElementaryStepFails2) {
  auto coll = db.getCollection("reactions");
  Reaction reaction;
  reaction.link(coll);
  ID id;
  ASSERT_THROW(reaction.hasElementaryStep(id), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.hasElementarySteps(), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.getElementarySteps(), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.addElementaryStep(id), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.setElementarySteps({}), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.removeElementaryStep(id), Exceptions::MissingIDException);
  ASSERT_THROW(reaction.clearElementarySteps(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
