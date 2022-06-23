/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Exceptions.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {

/**
 * @class ExceptionsTest
 * @brief Comprises tests for the classes in Exceptions.h.
 * @test
 */
class ExceptionsTest : public Test {};

TEST_F(ExceptionsTest, DuplicateIDException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::DuplicateIDException();
  }
  catch (Exceptions::DuplicateIDException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, MissingTimestampException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::MissingTimestampException();
  }
  catch (Exceptions::MissingTimestampException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, MissingCredentialsException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::MissingCredentialsException();
  }
  catch (Exceptions::MissingCredentialsException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, MissingLinkedCollectionException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::MissingLinkedCollectionException();
  }
  catch (Exceptions::MissingLinkedCollectionException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, DatabaseDisconnectedException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::DatabaseDisconnectedException();
  }
  catch (Exceptions::DatabaseDisconnectedException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, MissingCollectionException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::MissingCollectionException();
  }
  catch (Exceptions::MissingCollectionException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, MissingIDException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::MissingIDException();
  }
  catch (Exceptions::MissingIDException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, IDNotFoundException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::IDNotFoundException();
  }
  catch (Exceptions::IDNotFoundException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, UnpopulatedObjectException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::UnpopulatedObjectException();
  }
  catch (Exceptions::UnpopulatedObjectException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, ObjectTypeMismatchException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::ObjectTypeMismatchException();
  }
  catch (Exceptions::ObjectTypeMismatchException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, MissingIdOrField_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::MissingIdOrField();
  }
  catch (Exceptions::MissingIdOrField& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, FieldException_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::FieldException();
  }
  catch (Exceptions::FieldException& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

TEST_F(ExceptionsTest, VersionMismatch_HasWhat) {
  std::string ret;
  try {
    throw Exceptions::VersionMismatch();
  }
  catch (Exceptions::VersionMismatch& e) {
    ret = e.what();
  }
  ASSERT_FALSE(ret.empty());
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
