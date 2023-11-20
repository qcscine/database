/**
 * @file ObjectPython.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include "Helpers.h"
#include <Database/Collection.h>
#include <Database/Objects/Object.h>
#include <Utils/Pybind.h>
#include <pybind11/chrono.h>

using namespace Scine::Database;

void init_id(pybind11::module& m) {
  // export ID class
  pybind11::class_<ID, std::unique_ptr<ID>> id(m, "ID",
                                               R"delim(
      A long unique identifier for a database object. Can be represented as a
      hexadecimal string and compared

      >>> first = ID()
      >>> second = ID()
      >>> first < second
      True
      >>> first < second.string()
      True
    )delim");
  id.def(pybind11::init<>());
  id.def(pybind11::init<std::string>(), pybind11::arg("id_str"), "Initialize from an ID string serialization");
  id.def("string", &ID::string, "Convert the identifier to string representation");
  id.def("__str__", &ID::string);
  id.def(
      "__eq__", [](const ID& a, const ID& b) { return a == b; }, pybind11::is_operator());
  id.def(
      "__eq__", [](const ID& a, const std::string& b) { return a.string() == b; }, pybind11::is_operator());
  id.def(
      "__gt__", [](const ID& a, const ID& b) { return a > b; }, pybind11::is_operator());
  id.def(
      "__gt__", [](const ID& a, const std::string& b) { return a.string() > b; }, pybind11::is_operator());
  id.def(
      "__lt__", [](const ID& a, const ID& b) { return a < b; }, pybind11::is_operator());
  id.def(
      "__lt__", [](const ID& a, const std::string& b) { return a.string() < b; }, pybind11::is_operator());
  id.def(
      "__ge__", [](const ID& a, const ID& b) { return a >= b; }, pybind11::is_operator());
  id.def(
      "__ge__", [](const ID& a, const std::string& b) { return a.string() >= b; }, pybind11::is_operator());
  id.def(
      "__le__", [](const ID& a, const ID& b) { return a <= b; }, pybind11::is_operator());
  id.def(
      "__le__", [](const ID& a, const std::string& b) { return a.string() <= b; }, pybind11::is_operator());
  id.def("__repr__", [](const ID& id) { return "scine_database.ID('" + id.string() + "')"; });
  id.def(pybind11::pickle(
      [](const ID& id) { //__getstate__
        return id.string();
      },
      [](const std::string& s) { // __setstate__
        /* Create a new C++ instance */
        ID id(s);
        return id;
      }));
}

void init_object(pybind11::class_<Object>& object) {
  object.doc() = R"delim(
    Base class for any object stored in a database

    Objects by data representation consist of optionally an ID and optionally a
    linked collection. IDs in MongoDB are unique within a collection only, so
    a database object must be represented both by a collection and an ID.

    So in principle there are four possible states for the data in this object.
    Only if both ID and collection are populated can database operations against
    existing data succeed. In all other states, data access and modification of
    existing data will raise exceptions.

    :example:
    >>> property = NumberProperty()  # Empty init: No ID, no collection
    >>> isinstance(property, Object)  # This is a derived Object class
    True
    >>> property.data  # Try to access database data
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    RuntimeError: Missing linked collection.
    >>> property.link(manager.get_collection("properties"))
    >>> property.data
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    RuntimeError: The Object is missing an ID to be used in this context.

    ..note: It is important to consider that no ``Object`` class caches data.
            All methods and properties read or write data from the database so
            it is important to consider this when writing code.
  )delim";
  object.def("exists", &Object::exists,
             R"delim(
      Checks if the object exists in the linked collection

      :raises RuntimeError: If no collection is linked or the object has no ID.
        See `link` and `has_link`.
    )delim");
  object.def("link", &Object::link, pybind11::arg("collection"),
             R"delim(
      Link the object to a collection. All calls to other functions will then
      try to edit/create a document for this object in the linked collection
      with the given/generated ID.

      :param collection: The collection to link this object to
    )delim");
  object.def("has_link", &Object::hasLink,
             R"delim(
      Returns whether the object is linked to a collection or not. If the
      object is unlinked, database interaction is limited and many interface
      functions can raise errors.
    )delim");
  object.def("detach", &Object::detach,
             R"delim(
      Unlinks the instance, removing the collection pointer. Subsequent calls
      attempting to alter database data cannot cause changes in the database.
    )delim");
  object.def("id", &Object::id, "Returns the ID of the object");
  object.def("get_id", &Object::id, "Returns the ID of the object");
  object.def("has_id", &Object::hasId, "Checks if the object has an ID");
  object.def("print", &Object::print, "Prints a JSON string of the object's contents");
  object.def("json", &Object::json, "Fetches the JSON representation of the object's contents");
  object.def("__str__", &Object::json);
  object.def("get_collection", &Object::collection);
  object.def("wipe", &Object::wipe, pybind11::arg("expect_presence") = false,
             R"delim(
      Remove the object from the linked collection in a database

      Will also remove the ID from this object, if one is present.

      :param expect_presence: Raise an error if the object does not exist in
        the database
    )delim");
  object.def("older_than", &Object::olderThan, pybind11::arg("other"), pybind11::arg("modification") = false,
             R"delim(
      Compares the database timestamps of two objects

      :param other: Object to compare timestamps against
      :param modification: Selects which timestamps to compare. If true,
        compares the last modified time. If false, compares the time of creation.

      :raises RuntimeError: If no collection is linked or the object has no ID.
        See `link` and `has_link`.
    )delim");
  object.def("created", &Object::created,
             R"delim(
      Fetches the time when an object was created in the database

      :raises RuntimeError: If no collection is linked or the object has no ID.
        See `link` and `has_link`.
    )delim");
  object.def("last_modified", &Object::lastModified,
             R"delim(
      Fetches the time when an object was last modified in the database

      :raises RuntimeError: If no collection is linked or the object has no ID.
        See `link` and `has_link`.
    )delim");
  object.def("has_created_timestamp", &Object::hasCreatedTimestamp);
  object.def("has_last_modified_timestamp", &Object::hasLastModifiedTimestamp);
  object.def("enable_analysis", &Object::enable_analysis);
  object.def("enable_exploration", &Object::enable_exploration);
  object.def("disable_analysis", &Object::disable_analysis);
  object.def("disable_exploration", &Object::disable_exploration);
  object.def("analyze", &Object::analyze);
  object.def("explore", &Object::explore);
  object.def("touch", &Object::touch,
             R"delim(
      Sets or updates the last modified timestamp of this object in the database

      :raises RuntimeError: If no collection is linked or the object has no ID.
        See `link` and `has_link`.
    )delim");
}
