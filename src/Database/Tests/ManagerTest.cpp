/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Version.h>
#include <gmock/gmock.h>

using namespace testing;
namespace Scine {
namespace Database {
namespace Tests {
/**
 * @class ManagerTest ManagerTest.cpp
 * @brief Comprises tests for the class Scine::Database::Manager.
 * @test
 */
struct ManagerTest : public Test {
  Credentials credentials{TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_ManagerTest"};
  Manager db;

  void SetUp() final {
    db.setCredentials(credentials);
    db.connect();
  }
  void TearDown() final {
    db.wipe();
  }
};

TEST_F(ManagerTest, ConnectionRefused_WrongCredentials) {
  Manager manager;
  Credentials testdb("THERE_AINT_NO_HOST_HERE", 27017, "unittest_db_AaBbCc");
  manager.setCredentials(testdb);
  ASSERT_THROW(manager.connect(), std::exception);
}

TEST_F(ManagerTest, CredentialGetter_Working) {
  Manager manager;
  Credentials testdb("THERE_AINT_NO_HOST_HERE", 27017, "unittest_db_AaBbCc");
  manager.setCredentials(testdb);
  auto copy = manager.getCredentials();
  ASSERT_EQ(testdb.hostname, copy.hostname);
  ASSERT_EQ(testdb.port, copy.port);
  ASSERT_EQ(testdb.databaseName, copy.databaseName);
  ASSERT_EQ(testdb.username, copy.username);
  ASSERT_EQ(testdb.password, copy.password);
  ASSERT_EQ(testdb.authDatabase, copy.authDatabase);
}

TEST_F(ManagerTest, CredentialComparison) {
  Credentials testdb("THERE_AINT_NO_HOST_HERE", 27017, "unittest_db_AaBbCc");
  Credentials testdb1("THERE_AINT_NO_HOST_HERE", 27017, "unittest_db_AaBbCc");
  Credentials testdb2("THERE_AINT_NO_OTHER_HOST_HERE", 27017, "unittest_db_AaBbCc");
  ASSERT_TRUE(testdb == testdb1);
  ASSERT_FALSE(testdb == testdb2);
  ASSERT_FALSE(testdb1 == testdb2);
}

TEST_F(ManagerTest, DatabaseName_Working) {
  Manager manager;
  Credentials testdb(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_AaBbCc");
  manager.setCredentials(testdb);
  manager.connect();
  manager.init();
  ASSERT_TRUE(manager.isConnected());
  ASSERT_EQ(manager.getDatabaseName(), "unittest_db_AaBbCc");
  // Check collections
  ASSERT_TRUE(manager.hasCollection("structures"));
  ASSERT_TRUE(manager.hasCollection("calculations"));
  ASSERT_TRUE(manager.hasCollection("properties"));
  ASSERT_TRUE(manager.hasCollection("compounds"));
  ASSERT_TRUE(manager.hasCollection("reactions"));
  ASSERT_TRUE(manager.hasCollection("elementary_steps"));
  ASSERT_TRUE(manager.hasCollection("flasks"));
  manager.wipe();
  // Swap DB name
  manager.setDatabaseName("unittest_db_DdEeFf");
  manager.init();
  ASSERT_NE(manager.getDatabaseName(), "unittest_db_AaBbCc");
  ASSERT_EQ(manager.getDatabaseName(), "unittest_db_DdEeFf");
  // Check collections
  ASSERT_TRUE(manager.hasCollection("structures"));
  ASSERT_TRUE(manager.hasCollection("calculations"));
  ASSERT_TRUE(manager.hasCollection("properties"));
  ASSERT_TRUE(manager.hasCollection("compounds"));
  ASSERT_TRUE(manager.hasCollection("reactions"));
  ASSERT_TRUE(manager.hasCollection("elementary_steps"));
  ASSERT_TRUE(manager.hasCollection("flasks"));
  manager.wipe();
}

TEST_F(ManagerTest, ConnectionCredentialsInitAndWipe) {
  Manager manager;
  Credentials testdb(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_AaBbCc");
  manager.setCredentials(testdb);
  manager.connect();
  manager.init();
  ASSERT_TRUE(manager.isConnected());
  // Check collections
  ASSERT_TRUE(manager.hasCollection("structures"));
  ASSERT_TRUE(manager.hasCollection("calculations"));
  ASSERT_TRUE(manager.hasCollection("properties"));
  ASSERT_TRUE(manager.hasCollection("compounds"));
  ASSERT_TRUE(manager.hasCollection("reactions"));
  ASSERT_TRUE(manager.hasCollection("elementary_steps"));
  ASSERT_TRUE(manager.hasCollection("flasks"));
  // Wipe and check again
  manager.wipe();
  ASSERT_FALSE(manager.hasCollection("structures"));
  ASSERT_FALSE(manager.hasCollection("calculations"));
  ASSERT_FALSE(manager.hasCollection("properties"));
  ASSERT_FALSE(manager.hasCollection("compounds"));
  ASSERT_FALSE(manager.hasCollection("reactions"));
  ASSERT_FALSE(manager.hasCollection("elementary_steps"));
  ASSERT_FALSE(manager.hasCollection("flasks"));
  // Init and check again
  manager.init();
  ASSERT_TRUE(manager.hasCollection("structures"));
  ASSERT_TRUE(manager.hasCollection("calculations"));
  ASSERT_TRUE(manager.hasCollection("properties"));
  ASSERT_TRUE(manager.hasCollection("compounds"));
  ASSERT_TRUE(manager.hasCollection("reactions"));
  ASSERT_TRUE(manager.hasCollection("elementary_steps"));
  ASSERT_TRUE(manager.hasCollection("flasks"));
  // Clean
  manager.wipe();
  manager.disconnect();
  ASSERT_FALSE(manager.isConnected());
}

TEST_F(ManagerTest, RemoteWipe) {
  Manager manager;
  Credentials testdb(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_AaBbCc");
  manager.setCredentials(testdb);
  manager.connect();
  manager.init();
  ASSERT_TRUE(manager.isConnected());
  // Check collections
  ASSERT_TRUE(manager.hasCollection("structures"));
  ASSERT_TRUE(manager.hasCollection("calculations"));
  ASSERT_TRUE(manager.hasCollection("properties"));
  ASSERT_TRUE(manager.hasCollection("compounds"));
  ASSERT_TRUE(manager.hasCollection("reactions"));
  ASSERT_TRUE(manager.hasCollection("elementary_steps"));
  ASSERT_TRUE(manager.hasCollection("flasks"));
  // Wipe and check again
  manager.disconnect();
  manager.wipe(true);
  manager.connect();
  ASSERT_FALSE(manager.hasCollection("structures"));
  ASSERT_FALSE(manager.hasCollection("calculations"));
  ASSERT_FALSE(manager.hasCollection("properties"));
  ASSERT_FALSE(manager.hasCollection("compounds"));
  ASSERT_FALSE(manager.hasCollection("reactions"));
  ASSERT_FALSE(manager.hasCollection("elementary_steps"));
  ASSERT_FALSE(manager.hasCollection("flasks"));
}

TEST_F(ManagerTest, ServerTime) {
  Manager manager;
  ASSERT_THROW(manager.serverTime(), Exceptions::DatabaseDisconnectedException);
  Credentials testdb(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_AaBbCc");
  manager.setCredentials(testdb);
  manager.connect();
  manager.serverTime();
}

TEST_F(ManagerTest, Reconnect) {
  Manager manager;
  Credentials testdb(TEST_MONGO_DB_IP, std::atoi(TEST_MONGO_DB_PORT), "unittest_db_AaBbCc");
  manager.setCredentials(testdb);
  manager.connect();
  manager.init();
  manager.disconnect();
  manager.connect();
}

TEST_F(ManagerTest, GetCollection) {
  db.init();
  auto structures = db.getCollection("structures");
}

TEST_F(ManagerTest, CheckVersion) {
  db.init();
  ASSERT_TRUE(db.versionMatchesWrapper());
}

TEST_F(ManagerTest, GetVersion) {
  db.init();
  auto t = db.getDBVersion();
  ASSERT_EQ(std::get<0>(t), Version::major);
  ASSERT_EQ(std::get<1>(t), Version::minor);
  ASSERT_EQ(std::get<2>(t), Version::patch);
}

TEST_F(ManagerTest, GetCollectionFails) {
  Manager manager;
  ASSERT_THROW(manager.getCollection("nopenopenope", true), Exceptions::DatabaseDisconnectedException);
  ASSERT_THROW(db.getCollection("nopenopenope", true), Exceptions::MissingCollectionException);
}

TEST_F(ManagerTest, ExpectedDBFails) {
  ASSERT_THROW(db.connect(true), Exceptions::MissingCollectionException);
}

TEST_F(ManagerTest, HasCollectionFails) {
  Manager manager;
  ASSERT_THROW(manager.hasCollection("nopenopenope"), Exceptions::DatabaseDisconnectedException);
}

TEST_F(ManagerTest, WipeFails) {
  Manager manager;
  ASSERT_THROW(manager.wipe(), Exceptions::DatabaseDisconnectedException);
}

TEST_F(ManagerTest, InitFails) {
  Manager manager;
  ASSERT_THROW(manager.init(), Exceptions::DatabaseDisconnectedException);
}

} /* namespace Tests */
} /* namespace Database */
} /* namespace Scine */
