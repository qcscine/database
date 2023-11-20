/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include <Database/Collection.h>
#include <Database/Manager.h>
#include <Utils/Pybind.h>
#include <pybind11/chrono.h>
#include <pybind11/operators.h>

using namespace Scine::Database;

void init_credentials(pybind11::module& m) {
  pybind11::class_<Credentials> credentials(m, "Credentials");
  credentials.def(pybind11::init<>());
  credentials.def(
      pybind11::init<std::string, int, std::string, std::string, std::string, std::string, std::string, bool, bool>(),
      pybind11::arg("ip"), pybind11::arg("port"), pybind11::arg("database"), pybind11::arg("username") = "",
      pybind11::arg("password") = "", pybind11::arg("auth_database") = "", pybind11::arg("replica_set") = "",
      pybind11::arg("ssl_enabled") = false, pybind11::arg("retry_writes") = false);
  credentials.def_readwrite("hostname", &Credentials::hostname);
  credentials.def_readwrite("port", &Credentials::port);
  credentials.def_readwrite("databaseName", &Credentials::databaseName);
  credentials.def_readwrite("database_name", &Credentials::databaseName);
  credentials.def_readwrite("username", &Credentials::username);
  credentials.def_readwrite("password", &Credentials::password);
  credentials.def_readwrite("auth_database", &Credentials::authDatabase);
  credentials.def_readwrite("authDatabase", &Credentials::authDatabase);
  credentials.def_readwrite("connection_timeout", &Credentials::connectionTimeout);
  credentials.def_readwrite("access_timeout", &Credentials::accessTimeout);
  credentials.def_readwrite("replica_set", &Credentials::replicaSet);
  credentials.def_readwrite("ssl_enabled", &Credentials::sslEnabled);
  credentials.def_readwrite("retry_writes", &Credentials::retryWrites);
  credentials.def(pybind11::self == pybind11::self);
  credentials.def(pybind11::self != pybind11::self);
  credentials.def(pybind11::pickle(
      [](const Credentials& c) { // __getstate__
        /* Return a tuple that fully encodes the state of the object */
        return pybind11::make_tuple(c.hostname, c.port, c.databaseName, c.username, c.password, c.authDatabase);
      },
      [](pybind11::tuple t) { // __setstate__
        if (t.size() != 6)
          throw std::runtime_error("Invalid state for Credentials!");

        /* Create a new C++ instance */
        Credentials c(t[0].cast<std::string>(), t[1].cast<int>(), t[2].cast<std::string>(), t[3].cast<std::string>(),
                      t[4].cast<std::string>(), t[5].cast<std::string>());
        return c;
      }));
}

void init_manager(pybind11::class_<Manager>& manager) {
  manager.def(pybind11::init<>());
  manager.def("set_credentials", &Manager::setCredentials);
  manager.def("get_credentials", pybind11::overload_cast<>(&Manager::getCredentials));
  manager.def_property("credentials", pybind11::overload_cast<>(&Manager::getCredentials), &Manager::setCredentials);
  manager.def_property("database_name", &Manager::getDatabaseName, &Manager::setDatabaseName);
  manager.def("set_database_name", &Manager::setDatabaseName);
  manager.def("get_database_name", &Manager::getDatabaseName);
  manager.def("set_uri", &Manager::setUri);
  manager.def("get_uri", &Manager::getUri);
  manager.def("clear_uri", &Manager::clearUri);
  manager.def("connect", &Manager::connect, pybind11::arg("expect_initialized_db") = false,
              pybind11::arg("connection_timeout") = 60, pybind11::arg("access_timeout") = 0,
              pybind11::arg("replica_set") = std::string{}, pybind11::arg("ssl_enabled") = false,
              pybind11::arg("retry_writes") = false);
  manager.def("disconnect", &Manager::disconnect);
  manager.def("has_credentials",
              Scine::Utils::deprecated(&Manager::hasCredentials, "Manager has default credentials now!"));
  manager.def("is_connected", &Manager::isConnected);
  manager.def_property_readonly("connected", &Manager::isConnected);
  manager.def("init", &Manager::init, pybind11::arg("more_indices") = true);
  manager.def("wipe", &Manager::wipe, pybind11::arg("remote") = false);
  manager.def("has_collection", &Manager::hasCollection);
  manager.def("server_time", &Manager::serverTime);
  manager.def("version_matches_wrapper", &Manager::versionMatchesWrapper);
  manager.def("get_db_version", &Manager::getDBVersion);
  manager.def("get_collection", &Manager::getCollection, pybind11::arg("name"), pybind11::arg("expectpresent") = true);
}
