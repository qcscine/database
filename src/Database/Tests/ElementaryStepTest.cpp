/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/ElementaryStep.h>
#include <Database/Objects/ReactionSide.h>
#include <Utils/Math/BSplines/ReactionProfileInterpolation.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class ElementaryStepTest ElementaryStepTest.cpp
 * @brief Comprises tests for the class Scine::Database::ElementaryStep.
 * @test
 */
class ElementaryStepTest : public Test {
 public:
  ElementaryStepTest() {
  }
  static Credentials credentials;
  static Manager db;

  static void SetUpTestCase() {
    db.setCredentials(credentials);
    db.connect();
    db.init();
  }
  static void TearDownTestCase() {
    // db.wipe();
  }
};
Credentials ElementaryStepTest::credentials(TEST_MONGO_DB_IP, 27017, "unittest_db_ElementaryStepTest");
Manager ElementaryStepTest::db;

TEST_F(ElementaryStepTest, Create) {
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  ASSERT_TRUE(step.hasId());
  ASSERT_EQ(std::get<0>(step.getReactants(SIDE::BOTH))[0], id1);
  ASSERT_EQ(std::get<1>(step.getReactants(SIDE::BOTH))[0], id2);
}

TEST_F(ElementaryStepTest, ReactantLHS) {
  // Basic setup
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2, id3, id4, id5, id6;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  ASSERT_TRUE(step.hasId());
  // Checks
  ASSERT_EQ(step.hasReactant(id1), SIDE::LHS);
  ASSERT_EQ(step.hasReactant(id2), SIDE::RHS);
  ASSERT_EQ(1, std::get<0>(step.hasReactants()));
  ASSERT_EQ(1, std::get<1>(step.hasReactants()));
  step.addReactant(id3, SIDE::LHS);
  ASSERT_EQ(step.hasReactant(id3), SIDE::LHS);
  ASSERT_EQ(2, std::get<0>(step.hasReactants()));
  ASSERT_EQ(1, std::get<1>(step.hasReactants()));
  auto reactants = step.getReactants(SIDE::LHS);
  ASSERT_EQ(std::get<0>(reactants)[0], id1);
  ASSERT_EQ(std::get<0>(reactants)[1], id3);
  ASSERT_EQ(std::get<1>(reactants).size(), 0);
  step.setReactants({id4, id5, id6}, SIDE::LHS);
  ASSERT_EQ(3, std::get<0>(step.hasReactants()));
  step.removeReactant(id5, SIDE::LHS);
  ASSERT_EQ(2, std::get<0>(step.hasReactants()));
  step.clearReactants(SIDE::LHS);
  ASSERT_EQ(0, std::get<0>(step.hasReactants()));
}

TEST_F(ElementaryStepTest, ReactantRHS) {
  // Basic setup
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2, id3, id4, id5, id6;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  ASSERT_TRUE(step.hasId());
  // Checks
  ASSERT_EQ(step.hasReactant(id1), SIDE::LHS);
  ASSERT_EQ(step.hasReactant(id2), SIDE::RHS);
  ASSERT_EQ(1, std::get<0>(step.hasReactants()));
  ASSERT_EQ(1, std::get<1>(step.hasReactants()));
  step.addReactant(id3, SIDE::RHS);
  ASSERT_EQ(step.hasReactant(id3), SIDE::RHS);
  ASSERT_EQ(1, std::get<0>(step.hasReactants()));
  ASSERT_EQ(2, std::get<1>(step.hasReactants()));
  auto reactants = step.getReactants(SIDE::RHS);
  ASSERT_EQ(std::get<1>(reactants)[0], id2);
  ASSERT_EQ(std::get<1>(reactants)[1], id3);
  ASSERT_EQ(std::get<0>(reactants).size(), 0);
  step.setReactants({id4, id5, id6}, SIDE::RHS);
  ASSERT_EQ(3, std::get<1>(step.hasReactants()));
  step.removeReactant(id5, SIDE::RHS);
  ASSERT_EQ(2, std::get<1>(step.hasReactants()));
  step.clearReactants(SIDE::RHS);
  ASSERT_EQ(0, std::get<1>(step.hasReactants()));
}

TEST_F(ElementaryStepTest, ReactantBOTH) {
  // Basic setup
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2, id3, id4, id5, id6;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  ASSERT_TRUE(step.hasId());
  // Checks
  ASSERT_EQ(step.hasReactant(id1), SIDE::LHS);
  ASSERT_EQ(step.hasReactant(id2), SIDE::RHS);
  ASSERT_EQ(1, std::get<0>(step.hasReactants()));
  ASSERT_EQ(1, std::get<1>(step.hasReactants()));
  step.addReactant(id3, SIDE::BOTH);
  ASSERT_EQ(step.hasReactant(id3), SIDE::BOTH);
  ASSERT_EQ(2, std::get<0>(step.hasReactants()));
  ASSERT_EQ(2, std::get<1>(step.hasReactants()));
  auto reactants = step.getReactants(SIDE::BOTH);
  ASSERT_EQ(std::get<0>(reactants)[0], id1);
  ASSERT_EQ(std::get<0>(reactants)[1], id3);
  ASSERT_EQ(std::get<1>(reactants)[0], id2);
  ASSERT_EQ(std::get<1>(reactants)[1], id3);
  step.setReactants({id4, id5, id6}, SIDE::BOTH);
  ASSERT_EQ(3, std::get<0>(step.hasReactants()));
  ASSERT_EQ(3, std::get<1>(step.hasReactants()));
  step.removeReactant(id5, SIDE::BOTH);
  ASSERT_EQ(2, std::get<0>(step.hasReactants()));
  ASSERT_EQ(2, std::get<1>(step.hasReactants()));
  step.clearReactants(SIDE::BOTH);
  ASSERT_EQ(0, std::get<0>(step.hasReactants()));
  ASSERT_EQ(0, std::get<1>(step.hasReactants()));
}

TEST_F(ElementaryStepTest, ReactantFails1) {
  ElementaryStep step;
  ID id;
  ASSERT_THROW(step.hasReactant(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.hasReactants(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.getReactants(SIDE::BOTH), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.addReactant(id, SIDE::BOTH), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.setReactants({}, SIDE::BOTH), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.removeReactant(id, SIDE::BOTH), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.clearReactants(SIDE::BOTH), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ElementaryStepTest, ReactantFails2) {
  auto coll = db.getCollection("elementary_steps");
  ElementaryStep step;
  step.link(coll);
  ID id;
  ASSERT_THROW(step.hasReactant(id), Exceptions::MissingIDException);
  ASSERT_THROW(step.hasReactants(), Exceptions::MissingIDException);
  ASSERT_THROW(step.getReactants(SIDE::BOTH), Exceptions::MissingIDException);
  ASSERT_THROW(step.addReactant(id, SIDE::BOTH), Exceptions::MissingIDException);
  ASSERT_THROW(step.setReactants({}, SIDE::BOTH), Exceptions::MissingIDException);
  ASSERT_THROW(step.removeReactant(id, SIDE::BOTH), Exceptions::MissingIDException);
  ASSERT_THROW(step.clearReactants(SIDE::BOTH), Exceptions::MissingIDException);
}

TEST_F(ElementaryStepTest, Type) {
  // Setup
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  // Checks
  ASSERT_EQ(ElementaryStepType::REGULAR, step.getType());
  step.setType(ElementaryStepType::BARRIERLESS);
  ASSERT_EQ(ElementaryStepType::BARRIERLESS, step.getType());
  ASSERT_NE(ElementaryStepType::REGULAR, step.getType());
}

TEST_F(ElementaryStepTest, TypeFails1) {
  ElementaryStep step;
  ASSERT_THROW(step.getType(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.setType(ElementaryStepType::REGULAR), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ElementaryStepTest, TypeFails2) {
  auto coll = db.getCollection("elementary_steps");
  ElementaryStep step;
  step.link(coll);
  ASSERT_THROW(step.getType(), Exceptions::MissingIDException);
  ASSERT_THROW(step.setType(ElementaryStepType::REGULAR), Exceptions::MissingIDException);
}

TEST_F(ElementaryStepTest, TransitionState) {
  // Setup
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2, id3;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  ASSERT_TRUE(step.hasId());
  // Checks
  ASSERT_FALSE(step.hasTransitionState());
  step.setTransitionState(id3);
  ASSERT_TRUE(step.hasTransitionState());
  ASSERT_EQ(step.getTransitionState(), id3);
  step.clearTransitionState();
  ASSERT_FALSE(step.hasTransitionState());
}

TEST_F(ElementaryStepTest, TransitionStateFails1) {
  ElementaryStep step;
  ID id;
  ASSERT_THROW(step.hasTransitionState(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.setTransitionState(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.getTransitionState(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.clearTransitionState(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ElementaryStepTest, TransitionStateFails2) {
  auto coll = db.getCollection("elementary_steps");
  ElementaryStep step;
  step.link(coll);
  ID id;
  ASSERT_THROW(step.hasTransitionState(), Exceptions::MissingIDException);
  ASSERT_THROW(step.setTransitionState(id), Exceptions::MissingIDException);
  ASSERT_THROW(step.getTransitionState(), Exceptions::MissingIDException);
  ASSERT_THROW(step.clearTransitionState(), Exceptions::MissingIDException);
}

TEST_F(ElementaryStepTest, Reaction) {
  // Setup
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2, id3;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  ASSERT_TRUE(step.hasId());
  // Checks
  ASSERT_FALSE(step.hasReaction());
  step.setReaction(id3);
  ASSERT_TRUE(step.hasReaction());
  ASSERT_EQ(step.getReaction(), id3);
  step.clearReaction();
  ASSERT_FALSE(step.hasReaction());
}

TEST_F(ElementaryStepTest, ReactionFails1) {
  ElementaryStep step;
  ID id;
  ASSERT_THROW(step.hasReaction(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.setReaction(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.getReaction(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.clearReaction(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ElementaryStepTest, ReactionFails2) {
  auto coll = db.getCollection("elementary_steps");
  ElementaryStep step;
  step.link(coll);
  ID id;
  ASSERT_THROW(step.hasReaction(), Exceptions::MissingIDException);
  ASSERT_THROW(step.setReaction(id), Exceptions::MissingIDException);
  ASSERT_THROW(step.getReaction(), Exceptions::MissingIDException);
  ASSERT_THROW(step.clearReaction(), Exceptions::MissingIDException);
}

TEST_F(ElementaryStepTest, Spline) {
  // Setup
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2, id3;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  ASSERT_TRUE(step.hasId());
  // Checks
  Eigen::VectorXd knots(3);
  knots << 0.0, 0.5, 1.0;
  Eigen::MatrixXd data(3, 4);
  // clang-format off
  data << 1.0, 2.0, 3.0, 4.0,
          1.0, 2.0, 3.0, 4.0,
          1.0, 2.0, 3.0, 4.0;
  // clang-format on
  Utils::BSplines::TrajectorySpline ref({Utils::ElementType::H}, knots, data, 0.123);
  ASSERT_FALSE(step.hasSpline());
  step.setSpline(ref);
  ASSERT_TRUE(step.hasSpline());
  auto spline = step.getSpline();
  ASSERT_EQ(spline.elements[0], ref.elements[0]);
  ASSERT_EQ(spline.tsPosition, ref.tsPosition);
  for (unsigned int i = 0; i < 3; i++) {
    ASSERT_EQ(spline.knots[i], ref.knots[i]);
    for (unsigned int j = 0; j < 4; j++) {
      ASSERT_EQ(spline.data(i, j), ref.data(i, j));
    }
  }
  step.clearSpline();
  ASSERT_FALSE(step.hasSpline());
}

TEST_F(ElementaryStepTest, SplineFails1) {
  ElementaryStep step;
  Utils::BSplines::TrajectorySpline spline({}, {}, {});
  ASSERT_THROW(step.hasSpline(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.setSpline(spline), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.getSpline(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.clearSpline(), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ElementaryStepTest, SplineFails2) {
  auto coll = db.getCollection("elementary_steps");
  ElementaryStep step;
  step.link(coll);
  Utils::BSplines::TrajectorySpline spline({}, {}, {});
  ASSERT_THROW(step.hasSpline(), Exceptions::MissingIDException);
  ASSERT_THROW(step.setSpline(spline), Exceptions::MissingIDException);
  ASSERT_THROW(step.getSpline(), Exceptions::MissingIDException);
  ASSERT_THROW(step.clearSpline(), Exceptions::MissingIDException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
