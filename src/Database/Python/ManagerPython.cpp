/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include <Database/Collection.h>
#include <Database/Manager.h>
#include <Utils/Pybind.h>
#include <pybind11/chrono.h>

using namespace Scine::Database;

void init_credentials(pybind11::module& m) {
  pybind11::class_<Credentials> credentials(m, "Credentials");
  credentials.def(pybind11::init<>());
  credentials.def(pybind11::init<std::string, int, std::string>(), pybind11::arg("ip"), pybind11::arg("port"),
                  pybind11::arg("database"));
  credentials.def_readwrite("hostname", &Credentials::hostname);
  credentials.def_readwrite("port", &Credentials::port);
  credentials.def_readwrite("databaseName", &Credentials::databaseName);
  credentials.def_readwrite("database_name", &Credentials::databaseName);
  credentials.def_readwrite("username", &Credentials::username);
  credentials.def_readwrite("password", &Credentials::password);
  credentials.def_readwrite("auth_database", &Credentials::authDatabase);
  credentials.def_readwrite("authDatabase", &Credentials::authDatabase);
}

void init_manager(pybind11::class_<Manager>& manager) {
  manager.def(pybind11::init<>());
  manager.def("set_credentials", &Manager::setCredentials);
  manager.def("get_credentials", pybind11::overload_cast<>(&Manager::getCredentials));
  manager.def_property("credentials", pybind11::overload_cast<>(&Manager::getCredentials), &Manager::setCredentials);
  manager.def_property("database_name", &Manager::getDatabaseName, &Manager::setDatabaseName);
  manager.def("set_database_name", &Manager::setDatabaseName);
  manager.def("get_database_name", &Manager::getDatabaseName);
  manager.def("connect", &Manager::connect, pybind11::arg("expect_initialized_db") = false,
              pybind11::arg("connection_timeout") = 60, pybind11::arg("access_timeout") = 0);
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
