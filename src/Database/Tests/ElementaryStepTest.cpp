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
#include <Database/Objects/ReactionEnums.h>
#include <Database/Objects/Structure.h>
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
Credentials ElementaryStepTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_ElementaryStepTest");
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
  ASSERT_NO_THROW(step.getBarrierFromSpline());
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

TEST_F(ElementaryStepTest, Path) {
  // Setup
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2, id3;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  ASSERT_TRUE(step.hasId());
  // Checks
  ASSERT_FALSE(step.hasPath());
  ASSERT_FALSE(step.hasStructureInPath(id3));
  step.setPath({id1, id2, id3});
  ASSERT_TRUE(step.hasPath());
  ASSERT_TRUE(step.hasStructureInPath(id1));
  ASSERT_TRUE(step.hasStructureInPath(id2));
  ASSERT_TRUE(step.hasStructureInPath(id3));
  auto path = step.getPath();
  ASSERT_EQ(path[0], id1);
  ASSERT_EQ(path[1], id2);
  ASSERT_EQ(path[2], id3);
  step.clearPath();
  ASSERT_FALSE(step.hasPath());
}

TEST_F(ElementaryStepTest, PathFails1) {
  ElementaryStep step;
  ID id;
  ASSERT_THROW(step.hasPath(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.setPath({id}), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.getPath(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.clearPath(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.hasStructureInPath(id), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.getPath(db), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ElementaryStepTest, PathFails2) {
  auto coll = db.getCollection("elementary_steps");
  ElementaryStep step;
  step.link(coll);
  ID id;
  ASSERT_THROW(step.hasPath(), Exceptions::MissingIDException);
  ASSERT_THROW(step.setPath({id}), Exceptions::MissingIDException);
  ASSERT_THROW(step.getPath(), Exceptions::MissingIDException);
  ASSERT_THROW(step.clearPath(), Exceptions::MissingIDException);
  ASSERT_THROW(step.hasStructureInPath(id), Exceptions::MissingIDException);
  ASSERT_THROW(step.getPath(db), Exceptions::MissingIDException);
}

TEST_F(ElementaryStepTest, GetBarrierFromSpline) {
  // Setup
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2, id3;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  ASSERT_TRUE(step.hasId());
  // Checks
  Eigen::VectorXd knots(23);
  const double lhsEnergy = -31.682317;
  const double rhsEnergy = -31.678617;
  const double tsEnergy = -31.6597602759549;
  knots << 0.0, 0.0476190476190476, 0.0952380952380952, 0.142857142857143, 0.19047619047619, 0.238095238095238,
      0.285714285714286, 0.333333333333333, 0.380952380952381, 0.410853545626704, 0.428571428571429, 0.476190476190476,
      0.523809523809524, 0.571428571428571, 0.619047619047619, 0.666666666666667, 0.714285714285714, 0.761904761904762,
      0.80952380952381, 0.857142857142857, 0.904761904761905, 0.952380952380953, 1.0;
  Eigen::MatrixXd data(23, 4);
  data << -31.682317, 1.0, 2.0, 3.0, -31.6821136306482, 1.0, 2.0, 3.0, -31.6819803530015, 1.0, 2.0, 3.0,
      -31.6818292697806, 1.0, 2.0, 3.0, -31.6816460072194, 1.0, 2.0, 3.0, -31.6814279840998, 1.0, 2.0, 3.0,
      -31.680507474007, 1.0, 2.0, 3.0, -31.6771058158189, 1.0, 2.0, 3.0, -31.6710393590044, 1.0, 2.0, 3.0,
      -31.6597602759549, 1.0, 2.0, 3.0, -31.6587044902872, 1.0, 2.0, 3.0, -31.6580156229621, 1.0, 2.0, 3.0,
      -31.6574908028447, 1.0, 2.0, 3.0, -31.6572971835018, 1.0, 2.0, 3.0, -31.6552530639117, 1.0, 2.0, 3.0,
      -31.6575674276927, 1.0, 2.0, 3.0, -31.6592674998434, 1.0, 2.0, 3.0, -31.6636869549772, 1.0, 2.0, 3.0,
      -31.6735945247416, 1.0, 2.0, 3.0, -31.6768490547652, 1.0, 2.0, 3.0, -31.6777824333401, 1.0, 2.0, 3.0,
      -31.6784301522894, 1.0, 2.0, 3.0, -31.678617, 1.0, 2.0, 3.0;
  const double tsPosition = 0.410853545626704;
  Utils::BSplines::TrajectorySpline ref({Utils::ElementType::H}, knots, data, tsPosition);
  ASSERT_FALSE(step.hasSpline());
  step.setSpline(ref);
  ASSERT_TRUE(step.hasSpline());
  const auto barriers = step.getBarrierFromSpline();
  ASSERT_NEAR(std::get<0>(barriers), tsEnergy - lhsEnergy, 1e-6);
  ASSERT_NEAR(std::get<1>(barriers), tsEnergy - rhsEnergy, 1e-6);
  ASSERT_NEAR(std::get<0>(ref.evaluate(0.0, 3)), lhsEnergy, 1e-6);
  ASSERT_NEAR(std::get<0>(ref.evaluate(1.0, 3)), rhsEnergy, 1e-6);
  // The spline makes the TS energy rather inaccurate. In this case, the error is around 0.5 mEh.
  ASSERT_NEAR(std::get<0>(ref.evaluate(tsPosition, 3)), tsEnergy, 1e-3);
}

TEST_F(ElementaryStepTest, IdxMaps) {
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  step.link(coll);

  // Only lhs to rhs mapping
  ASSERT_FALSE(step.hasIdxMap(ElementaryStep::IdxMapType::LHS_RHS));
  std::vector<int> lhsRhs{1, 2, 0, 3}; // (A, B, C, D) (lhs) -> (C, A, B, D) (rhs)
  step.addIdxMaps(lhsRhs);
  ASSERT_TRUE(step.hasIdxMap(ElementaryStep::IdxMapType::LHS_RHS));
  ASSERT_TRUE(step.hasIdxMap(ElementaryStep::IdxMapType::RHS_LHS));
  ASSERT_FALSE(step.hasIdxMap(ElementaryStep::IdxMapType::LHS_TS));
  ASSERT_FALSE(step.hasIdxMap(ElementaryStep::IdxMapType::TS_LHS));
  ASSERT_FALSE(step.hasIdxMap(ElementaryStep::IdxMapType::TS_RHS));
  ASSERT_FALSE(step.hasIdxMap(ElementaryStep::IdxMapType::RHS_TS));
  ASSERT_TRUE(step.getIdxMap(ElementaryStep::IdxMapType::LHS_RHS) == lhsRhs);
  step.removeIdxMaps();
  ASSERT_FALSE(step.hasIdxMap(ElementaryStep::IdxMapType::LHS_RHS));
  ASSERT_FALSE(step.hasIdxMap(ElementaryStep::IdxMapType::RHS_LHS));

  // lhs to rhs and lhs to ts mapping
  std::vector<int> lhsTs{1, 3, 0, 2}; // (A, B, C, D) (lhs) -> (C, A, D, B) (ts)
  step.addIdxMaps(lhsRhs, lhsTs);
  ASSERT_TRUE(step.hasIdxMap(ElementaryStep::IdxMapType::LHS_RHS));
  ASSERT_TRUE(step.hasIdxMap(ElementaryStep::IdxMapType::RHS_LHS));
  ASSERT_TRUE(step.getIdxMap(ElementaryStep::IdxMapType::LHS_RHS) == lhsRhs);
  ASSERT_TRUE(step.getIdxMap(ElementaryStep::IdxMapType::LHS_TS) == lhsTs);
  // Reversed maps
  ASSERT_TRUE(step.hasIdxMap(ElementaryStep::IdxMapType::LHS_TS));
  ASSERT_TRUE(step.hasIdxMap(ElementaryStep::IdxMapType::TS_LHS));
  std::vector<int> rhsLhs{2, 0, 1, 3}; //  (C, A, B, D) (rhs) --> (A, B, C, D) (lhs)
  std::vector<int> tsLhs{2, 0, 3, 1};  //  (C, A, D, B) (ts) --> (A, B, C, D) (lhs)
  ASSERT_TRUE(step.getIdxMap(ElementaryStep::IdxMapType::RHS_LHS) == rhsLhs);
  ASSERT_TRUE(step.getIdxMap(ElementaryStep::IdxMapType::TS_LHS) == tsLhs);
  // ts rhs mappings can be retrieved from combination of lhs-rhs and lhs-ts
  ASSERT_TRUE(step.hasIdxMap(ElementaryStep::IdxMapType::TS_RHS));
  ASSERT_TRUE(step.hasIdxMap(ElementaryStep::IdxMapType::RHS_TS));
  std::vector<int> tsRhs{0, 1, 3, 2}; // (C, A, D, B) (ts) -> (C, A, B, D) (rhs)
  std::vector<int> rhsTs{0, 1, 3, 2}; // (C, A, B, D) (rhs) -> (C, A, D, B) (ts)
  ASSERT_TRUE(step.getIdxMap(ElementaryStep::IdxMapType::TS_RHS) == tsRhs);
  ASSERT_TRUE(step.getIdxMap(ElementaryStep::IdxMapType::RHS_TS) == rhsTs);
  step.removeIdxMaps();
  ASSERT_FALSE(step.hasIdxMap(ElementaryStep::IdxMapType::LHS_RHS));
  ASSERT_FALSE(step.hasIdxMap(ElementaryStep::IdxMapType::LHS_TS));
}

TEST_F(ElementaryStepTest, IdxMapsCollectionFails) {
  ElementaryStep step;
  std::vector<int> lhsRhs{1, 2, 0, 3}; // (A, B, C, D) (lhs) -> (C, A, B, D) (rhs)
  std::vector<int> lhsTs{1, 3, 0, 2};  // (A, B, C, D) (lhs) -> (C, A, D, B) (ts)
  ASSERT_THROW(step.hasIdxMap(ElementaryStep::IdxMapType::LHS_RHS), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.addIdxMaps(lhsRhs), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.addIdxMaps(lhsRhs, lhsRhs), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.removeIdxMaps(), Exceptions::MissingLinkedCollectionException);
  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::LHS_RHS), Exceptions::MissingLinkedCollectionException);
}

TEST_F(ElementaryStepTest, IdxMapsMissingFieldFails) {
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  step.link(coll);

  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::LHS_RHS), Exceptions::MissingIdOrField);
  std::vector<int> lhsRhs{1, 2, 0, 3}; // (A, B, C, D) (lhs) -> (C, A, B, D) (rhs)
  step.addIdxMaps(lhsRhs);
  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::LHS_TS), Exceptions::MissingIdOrField);
  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::TS_LHS), Exceptions::MissingIdOrField);
  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::TS_RHS), Exceptions::MissingIdOrField);
  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::RHS_TS), Exceptions::MissingIdOrField);
}

TEST_F(ElementaryStepTest, IdxMapsInvalidMapFails) {
  auto coll = db.getCollection("elementary_steps");
  ID id1, id2;
  ElementaryStep step = ElementaryStep::create({id1}, {id2}, coll);
  step.link(coll);
  std::vector<int> tooHigh{1, 3, 7, 2};
  std::vector<int> lhsTs{1, 3, 0, 2};
  step.addIdxMaps(tooHigh, lhsTs);
  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::RHS_LHS), ElementaryStep::InvalidIdxMapException);
  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::TS_RHS), ElementaryStep::InvalidIdxMapException);
  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::RHS_TS), ElementaryStep::InvalidIdxMapException);
  std::vector<int> differentSize{0, 1, 2};
  step.addIdxMaps(differentSize, lhsTs);
  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::TS_RHS), ElementaryStep::InvalidIdxMapException);
  ASSERT_THROW(step.getIdxMap(ElementaryStep::IdxMapType::RHS_TS), ElementaryStep::InvalidIdxMapException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
