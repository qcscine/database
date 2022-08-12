/**
 * @file Manager.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_MANAGER_H_
#define DATABASE_MANAGER_H_

/* External Includes */
#include <chrono>
#include <memory>
#include <string>

namespace mongocxx {
inline namespace v_noabi {
class client;
}
} // namespace mongocxx

namespace Scine {
namespace Database {

class Collection;

/**
 * @class Credentials Manager.h
 * @brief A class comprising all the needed credentials for a connection
 *        to a database.
 */
class Credentials {
 public:
  Credentials() = default;
  /**
   * @brief Construct a new Credentials object.
   * @param hostname The name of the host running the database.
   * @param port     The port the database listens on.
   * @param databaseName The name of the database on the database server.
   * @param username The username, if required.
   * @param password The password, if required.
   * @param authDatabase The authentication database, if authentication is required.
   */
  Credentials(std::string hostname, int port, std::string databaseName, std::string username = std::string{},
              std::string password = std::string{}, std::string authDatabase = std::string{});
  /// @brief The name of the host running the database.
  std::string hostname = "localhost";
  /// @brief The port the database listens on.
  int port = 27017;
  /// @brief The name of the database on the database server.
  std::string databaseName = "scine";
  /// @brief The username, if required.
  std::string username;
  /// @brief The password, if required.
  std::string password;
  /// @brief The authentication database, if authentication is required.
  std::string authDatabase;

  bool operator==(const Credentials& rhs) const {
    return this->hostname == rhs.hostname && this->port == rhs.port && this->databaseName == rhs.databaseName &&
           this->username == rhs.username && this->password == rhs.password && this->authDatabase == rhs.authDatabase;
  }

  bool operator!=(const Credentials& rhs) const {
    return !(*this == rhs);
  }
};

/**
 * @class Manager Manager.h
 * @brief A class managing access to a server and the databases on it.
 */
class Manager {
 public:
  /// @brief Constructor.
  Manager();
  /**
   * @brief Destructor.
   *
   * Non-default to handle disconnection from the database gracefully.
   */
  ~Manager();
  /**
   * @brief Set the Credentials.
   *
   * @note Will not trigger a disconnect if a database is already connected.
   *
   * @param credentials The credentials.
   */
  void setCredentials(Credentials credentials);
  /**
   * @brief Connect to the database using the current credentials.
   *
   * Values of zero disable the respective timeout.
   * For a better understanding of these timeouts see:
   *  https://docs.mongodb.com/manual/reference/connection-string/
   *
   * @param expectContent     If true expects an initialized DB to be present.
   * @param connectionTimeout The time in seconds for the initial connection
   *                          attempt to time out.
   * @param accessTimeout     The time in seconds for each action on the DB to
   *                          time out.
   *
   * Will disconnect first if there is a connection present.
   */
  void connect(bool expectContent = false, unsigned int connectionTimeout = 60, unsigned int accessTimeout = 0);
  /**
   * @brief Disconnect the current connection.
   * @throws MissingCredentialsException Thrown if no credentials are set.
   */
  void disconnect();
  /**
   * @brief Checks if there are credentials present.
   * @return true If there are credentials stored.
   * @return false If there are no credentials stored.
   */
  bool hasCredentials() const;
  //! Getter for the current credentials
  Credentials& getCredentials();
  /**
   * @brief Getter for the current credentials.
   * @return Credentials The current credentials.
   */
  const Credentials& getCredentials() const;
  /**
   * @brief Get the current database name.
   *
   * @note A server can hold multiple databases, hence, it is possible to
   *   switch the access to them without disconnecting and resetting the entire
   *   set of credentials.
   *
   * @return std::string The name of the database.
   */
  std::string getDatabaseName() const;
  /**
   * @brief Set the name of the current database to work on.
   *
   * @param name The name of the new database to work on.
   *
   * @parblock @note A server can hold multiple databases. It is possible to
   *   switch the access to them without disconnecting and resetting the entire
   *   set of credentials. @endparblock
   *
   * @parblock @note Switching this name does not invalidate all collection
   *   instances previously generated from a database with a different name.
   *   @endparblock
   *
   * @note Giving a name of an non-existing database will generate it.
   */
  void setDatabaseName(std::string name);
  /**
   * @brief Checks if there is an open connection.
   * @return true If there is a connection to a database.
   * @return false If there is no current connection to a database.
   */
  bool isConnected() const;
  /**
   * @brief Initializes the standard set of collections.
   *
   * Existing collections will be skipped and will not be cleared/wiped.
   * @param moreIndices If true, sets a lot of important calculation fields to
   *                    to be indexed by the database, improving lookup speeds,
   *                    but increasing resource consumption.
   * @throws DatabaseDisconnectedException Thrown if no database is connected.
   */
  void init(bool moreIndices = true);
  /**
   * @brief Clears all data from the connected database.
   *
   * All data in all collections will be lost upon calling this function!
   *
   * @param remote Setting remote to true allows to wipe a database without
   *               prior connection via the connect() function.
   *               This allows wiping databases that have missmatching version.
   *
   * @note Because this function drops all tables in a remote database and does
   *   simply clear the local temporary storage of runtime data, it is not
   *   called clear(), even though this name would also be fitting.
   *
   * @throws DatabaseDisconnectedException Thrown if no database is connected.
   */
  void wipe(bool remote = false);
  /**
   * @brief Check if a collection with a given name is available.
   *
   * Asserts a connection is present.
   *
   * @throws DatabaseDisconnectedException Thrown if no database is connected.
   *
   * @param name The name of the collection to check for.
   * @return true If the collection is present.
   * @return false  If the collection is not present.
   */
  bool hasCollection(const std::string& name) const;
  /**
   * @brief Get the Collection by name.
   *
   * Creates the collection if it is not present, unless the expectPresent flag
   * is set.
   *
   * @throws DatabaseDisconnectedException Thrown if no database is connected.
   * @throws MissingCollectionException Thrown if expectPresent is true and the collection is missing.
   *
   * @param name The name of the collection.
   * @param expectPresent Do not create new collections.
   * @return std::shared_ptr<Collection> The collection.
   */
  std::shared_ptr<Collection> getCollection(const std::string& name, bool expectPresent = true) const;
  /**
   * @brief The current servertime (UTC).
   * @throws DatabaseDisconnectedException Thrown if no database is connected.
   * @return std::chrono::system_clock::time_point The current server time.
   */
  std::chrono::system_clock::time_point serverTime() const;
  /**
   * @brief Check if the wrapper version and server version match.
   * @return true  If minor and major version number are identical.
   * @return false If more than the patch version mismatch.
   */
  bool versionMatchesWrapper();
  /**
   * @brief Get the version of the SCINE database running on the server.
   * @return std::tuple<unsigned int, unsigned int, unsigned int> The version tuple in order: major.minor.patch.
   */
  std::tuple<int, int, int> getDBVersion();

 private:
  Credentials _credentials;
  std::unique_ptr<mongocxx::v_noabi::client> _connection;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_MANAGER_H_ */
