/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Layout.h>
#include <Database/Manager.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class LayoutTest LayoutTest.cpp
 * @brief Comprises tests for the db layout
 * @test
 */
class LayoutTest : public Test {
 public:
  LayoutTest() {
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
Credentials LayoutTest::credentials(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_LayoutTest");
Manager LayoutTest::db;

TEST_F(LayoutTest, MapsComplete) {
  // Calculation status
  for (const auto& [strStatus, status] : Layout::EnumMaps::str2status) {
    ASSERT_FALSE(Layout::EnumMaps::status2str.find(status) == Layout::EnumMaps::status2str.end());
    ASSERT_TRUE(Layout::EnumMaps::status2str.at(status) == strStatus);
  }
  // Elementary step type
  for (const auto& [strType, type] : Layout::EnumMaps::str2estype) {
    ASSERT_FALSE(Layout::EnumMaps::estype2str.find(type) == Layout::EnumMaps::estype2str.end());
    ASSERT_TRUE(Layout::EnumMaps::estype2str.at(type) == strType);
  }
  // Structure labels
  for (const auto& [strLabel, label] : Layout::EnumMaps::str2label) {
    ASSERT_FALSE(Layout::EnumMaps::label2str.find(label) == Layout::EnumMaps::label2str.end());
    ASSERT_TRUE(Layout::EnumMaps::label2str.at(label) == strLabel);
  }
}

} // namespace Tests
} // namespace Database
} // namespace Scine
