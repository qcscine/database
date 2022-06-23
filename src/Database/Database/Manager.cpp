/**
 * @file Manager.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Includes*/
#include "Database/Manager.h"
#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Layout.h"
#include "Database/Objects/Calculation.h"
#include "Database/Objects/Compound.h"
#include "Database/Objects/ElementaryStep.h"
#include "Database/Objects/Property.h"
#include "Database/Objects/Reaction.h"
#include "Database/Objects/Structure.h"
#include "Database/Version.h"
/* External Includes */
#include <bsoncxx/builder/stream/document.hpp>
#include <cassert>
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

namespace Scine {
namespace Database {

Credentials::Credentials(std::string hostname, int port, std::string databaseName, std::string username,
                         std::string password, std::string authDatabase)
  : hostname(std::move(hostname)),
    port(port),
    databaseName(std::move(databaseName)),
    username(std::move(username)),
    password(std::move(password)),
    authDatabase(std::move(authDatabase)) {
}

Manager::Manager() {
  /* IMPORTANT: Before making any connections, one and only one instance of
   *            mongocxx::instance must exist.
   *            This instance must exist for the entirety of the program.
   */
  static mongocxx::instance inst;
}

Manager::~Manager() {
  if (this->isConnected())
    this->disconnect();
}

void Manager::connect(bool expectContent, unsigned int connectionTimeout, unsigned int accessTimeout) {
  if (!this->hasCredentials())
    throw Exceptions::MissingCredentialsException();
  if (this->isConnected())
    this->disconnect();
  std::string uri;
  if (!_credentials.username.empty() and !_credentials.password.empty()) {
    uri = "mongodb://" + _credentials.username + ":" + _credentials.password + "@" + _credentials.hostname + ":" +
          std::to_string(_credentials.port) + "/" + _credentials.authDatabase +
          "?socketTimeoutMS=" + std::to_string(accessTimeout * 1000) +
          "&connectTimeoutMS=" + std::to_string(connectionTimeout * 1000);
  }
  else {
    uri = "mongodb://" + _credentials.hostname + ":" + std::to_string(_credentials.port) +
          "/?socketTimeoutMS=" + std::to_string(accessTimeout * 1000) +
          "&connectTimeoutMS=" + std::to_string(connectionTimeout * 1000);
  }
  try {
    _connection = std::make_unique<mongocxx::client>(mongocxx::uri{uri});
  }
  catch (std::exception& err) {
    _connection = nullptr;
    throw err;
  }
  if (this->hasCollection("structures")) {
    if (!this->versionMatchesWrapper()) {
      throw Exceptions::VersionMismatch();
    }
  }
  else {
    if (expectContent) {
      throw Exceptions::MissingCollectionException();
    }
  }
}

void Manager::setCredentials(Credentials credentials) {
  _credentials = std::move(credentials);
}

std::string Manager::getDatabaseName() const {
  if (!this->hasCredentials())
    throw Exceptions::MissingCredentialsException();
  return _credentials.databaseName;
}

void Manager::setDatabaseName(std::string name) {
  if (!this->hasCredentials())
    throw Exceptions::MissingCredentialsException();
  _credentials.databaseName = std::move(name);
}

Credentials& Manager::getCredentials() {
  return _credentials;
}

const Credentials& Manager::getCredentials() const {
  return _credentials;
}

void Manager::disconnect() {
  _connection.reset(nullptr);
}
bool Manager::hasCredentials() const {
  return true;
}
bool Manager::isConnected() const {
  using namespace bsoncxx::builder::basic;
  if (!static_cast<bool>(_connection)) {
    return false;
  }

  try {
    auto db = this->_connection->database(_credentials.databaseName);
    db.run_command(make_document(kvp("isMaster", 1)));
    return true;
  }
  catch (std::exception& e) {
    return false;
  }
}
bool Manager::versionMatchesWrapper() {
  auto dbVersion = this->getDBVersion();
  if (std::get<0>(dbVersion) != Version::major || std::get<1>(dbVersion) != Version::minor) {
    return false;
  }
  return true;
}
std::tuple<int, int, int> Manager::getDBVersion() {
  if (!this->isConnected())
    throw Exceptions::DatabaseDisconnectedException();
  mongocxx::database db = _connection->database(_credentials.databaseName);
  auto meta = db.collection(Layout::InternalCollection::meta);
  auto selection = document{} << finalize;
  auto optional = meta.find_one(selection.view());
  if (!optional)
    return {0, 0, 0};
  auto view = optional.value().view();
  int major = view["version"]["major"].get_int32();
  int minor = view["version"]["minor"].get_int32();
  int patch = view["version"]["patch"].get_int32();
  return {major, minor, patch};
}
void Manager::init(bool moreIndices) {
  if (!this->isConnected()) {
    throw Exceptions::DatabaseDisconnectedException();
  }

  mongocxx::database db = _connection->database(_credentials.databaseName);

  /* Generate the default collections */
  for (const char* const name : Layout::DefaultCollection::all) {
    if (!db.has_collection(name)) {
      try {
        db.create_collection(name);
      }
      catch (std::runtime_error& e) {
        /* This can occur if init() is called twice simultaneously, but is
         * not an error
         */
      }
    }
  }

  if (!db.has_collection(Layout::InternalCollection::meta)) {
    try {
      db.create_collection(Layout::InternalCollection::meta);
    }
    catch (std::runtime_error& e) {
      /* This can occur if init() is called twice simultaneously, but is
       * not an error
       */
    }
    auto now = bsoncxx::types::b_date(std::chrono::system_clock::now());
    mongocxx::collection meta = db.collection(Layout::InternalCollection::meta);
    // clang-format off
    auto metaData = document{} << "_created" << now
                               << "version" << open_document
                                 << "major" << Version::major
                                 << "minor" << Version::minor
                                 << "patch" << Version::patch
                                 << close_document
                               << finalize;
    // clang-format on
    meta.insert_one(metaData.view());
  }

  // Generate Indices
  if (moreIndices) {
    mongocxx::collection structures = db.collection("structures");
    auto label = document{} << "label" << 1 << finalize;
    structures.create_index(std::move(label));

    mongocxx::collection calculations = db.collection("calculations");
    // clang-format off
    auto main = document{}
                           << "priority" << 1
                           << "job.cores" << -1 // to sort for highest cores
                           << "job.memory" << -1 // to sort for highest memory
                           << "job.disk" << -1 // to sort for highest disk
                           << "job.order" << 1
                           << "model.program" << 1
                           << "model.version" << 1
                           << "_objecttype" << 1 // used for return evaluation
                           << finalize;
    auto partial = document{} << "partialFilterExpression" << open_document
                                << "status" << open_document
                                  << "$eq" << "new"
                                  << close_document
                                << close_document
                              << "name" << "new_calc_partial"
                              << finalize;
    // clang-format on
    calculations.create_index(std::move(main), std::move(partial));
    auto status = document{} << "status" << 1 << finalize;
    calculations.create_index(std::move(status));
  }
}

void Manager::wipe(bool remote) {
  if (!this->isConnected() && !remote)
    throw Exceptions::DatabaseDisconnectedException();
  if (!remote) {
    this->_connection->database(_credentials.databaseName).drop();
  }
  else {
    if (!this->hasCredentials())
      throw Exceptions::MissingCredentialsException();
    std::string uri;
    if (!_credentials.username.empty() and !_credentials.password.empty()) {
      uri = "mongodb://" + _credentials.username + ":" + _credentials.password + "@" + _credentials.hostname + ":" +
            std::to_string(_credentials.port) + "/" + _credentials.authDatabase;
    }
    else {
      uri = "mongodb://" + _credentials.hostname + ":" + std::to_string(_credentials.port);
    }
    auto tmpConnection = std::make_unique<mongocxx::client>(mongocxx::uri{uri});
    tmpConnection->database(_credentials.databaseName).drop();
  }
}

bool Manager::hasCollection(const std::string& name) const {
  if (!this->isConnected())
    throw Exceptions::DatabaseDisconnectedException();
  auto db = this->_connection->database(_credentials.databaseName);
  return db.has_collection(name);
}

std::shared_ptr<Collection> Manager::getCollection(const std::string& name, bool expectPresent) const {
  if (!this->isConnected())
    throw Exceptions::DatabaseDisconnectedException();
  auto db = this->_connection->database(_credentials.databaseName);
  if (!db.has_collection(name)) {
    if (expectPresent) {
      throw Exceptions::MissingCollectionException();
    }

    db.create_collection(name);
  }
  return std::make_shared<Collection>(db.collection(name));
}

std::chrono::system_clock::time_point Manager::serverTime() const {
  if (!this->isConnected())
    throw Exceptions::DatabaseDisconnectedException();

  bsoncxx::builder::basic::document server_status{};
  server_status.append(bsoncxx::builder::basic::kvp("serverStatus", 1));
  bsoncxx::document::value output =
      this->_connection->database(_credentials.databaseName).run_command(server_status.extract());

  return {output.view()["localTime"].get_date()};
}

} /* namespace Database */
} /* namespace Scine */
