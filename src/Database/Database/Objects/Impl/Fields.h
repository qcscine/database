/**
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef INCLUDE_SCINE_DATABASE_OBJECTS_IMPL_FIELDS_H
#define INCLUDE_SCINE_DATABASE_OBJECTS_IMPL_FIELDS_H

#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Layout.h"
#include "Database/Objects/Model.h"
#include "Database/Objects/Object.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <iostream>
#include <mongocxx/collection.hpp>
#include <string>

namespace Scine {
namespace Database {
namespace Fields {

template<typename T>
struct Serialization {};

template<>
struct Serialization<std::string> {
  static const std::string& serialize(const std::string& data) {
    return data;
  }
  template<typename Variant>
  static boost::optional<std::string> deserialize(Variant data) {
    if (data.type() != bsoncxx::types::b_utf8::type_id) {
      return boost::none;
    }
    return static_cast<std::string>(data.get_utf8().value);
  }
};

template<>
struct Serialization<ID> {
  static bsoncxx::oid serialize(const ID& id) {
    return id.bsoncxx();
  }
  template<typename Variant>
  static boost::optional<ID> deserialize(Variant data) {
    if (data.type() != bsoncxx::types::b_oid::type_id) {
      return boost::none;
    }
    return ID{data.get_oid().value};
  }
};

template<>
struct Serialization<CalculationStatus> {
  static std::string serialize(const CalculationStatus status) {
    return Layout::EnumMaps::status2str.at(status);
  }
  template<typename Variant>
  static boost::optional<CalculationStatus> deserialize(Variant data) {
    const auto statusString = Serialization<std::string>::deserialize(data);
    if (!statusString) {
      return boost::none;
    }
    const auto findIter = Layout::EnumMaps::str2status.find(statusString.value());
    if (findIter == std::end(Layout::EnumMaps::str2status)) {
      return boost::none;
    }
    return findIter->second;
  }
};

template<>
struct Serialization<ElementaryStepType> {
  static std::string serialize(const ElementaryStepType estype) {
    return Layout::EnumMaps::estype2str.at(estype);
  }
  template<typename Variant>
  static boost::optional<ElementaryStepType> deserialize(Variant data) {
    const auto typeString = Serialization<std::string>::deserialize(data);
    if (!typeString) {
      return boost::none;
    }
    const auto findIter = Layout::EnumMaps::str2estype.find(typeString.value());
    if (findIter == std::end(Layout::EnumMaps::str2estype)) {
      return boost::none;
    }
    return findIter->second;
  }
};

template<>
struct Serialization<StructureLabel> {
  static std::string serialize(const StructureLabel status) {
    return Layout::EnumMaps::label2str.at(status);
  }
  template<typename Variant>
  static boost::optional<StructureLabel> deserialize(Variant data) {
    const auto statusString = Serialization<std::string>::deserialize(data);
    if (!statusString) {
      return boost::none;
    }
    const auto findIter = Layout::EnumMaps::str2label.find(statusString.value());
    if (findIter == std::end(Layout::EnumMaps::str2label)) {
      return boost::none;
    }
    return findIter->second;
  }
};

template<>
struct Serialization<double> {
  static double serialize(const double x) {
    return x;
  }
  template<typename Variant>
  static boost::optional<double> deserialize(Variant data) {
    if (data.type() != bsoncxx::types::b_double::type_id) {
      return boost::none;
    }
    return data.get_double().value;
  }
};

template<>
struct Serialization<bool> {
  static bool serialize(const bool x) {
    return x;
  }
  template<typename Variant>
  static boost::optional<bool> deserialize(Variant data) {
    if (data.type() != bsoncxx::types::b_bool::type_id) {
      return boost::none;
    }
    return data.get_bool().value;
  }
};

template<>
struct Serialization<int> {
  static int serialize(const int x) {
    return x;
  }
  template<typename Variant>
  static boost::optional<int> deserialize(Variant data) {
    if (data.type() == bsoncxx::types::b_int64::type_id) {
      const long x = data.get_int64().value;
      if (x > std::numeric_limits<int>::max()) {
        throw std::runtime_error("Tried to overflow a 64-bit integer into a 32-bit integer");
      }
      return static_cast<int>(x);
    }

    if (data.type() == bsoncxx::types::b_int32::type_id) {
      return data.get_int32().value;
    }

    return boost::none;
  }
};

template<>
struct Serialization<Model> {
  static bsoncxx::document::value serialize(const Model& model) {
    return model.toBson();
  }
  template<typename Variant>
  static boost::optional<Model> deserialize(Variant data) {
    if (data.type() != bsoncxx::types::b_document::type_id) {
      return boost::none;
    }
    return Model{data.get_document().view()};
  }
};

template<typename T>
struct Serialization<std::vector<T>> {
  static bsoncxx::array::value serialize(const std::vector<T>& vs) {
    bsoncxx::builder::basic::array array;
    for (const auto& v : vs) {
      array.append(Serialization<T>::serialize(v));
    }
    return array.extract();
  }
  template<typename Variant>
  static boost::optional<std::vector<T>> deserialize(Variant data) {
    if (data.type() != bsoncxx::types::b_array::type_id) {
      return boost::none;
    }

    std::vector<T> values;
    auto array = data.get_array().value;
    for (auto element : array) {
      try {
        values.push_back(Serialization<T>::deserialize(element).value());
      }
      catch (boost::bad_optional_access& e) {
        throw std::runtime_error("Failed to deserialize array subtype!");
      }
    }
    return values;
  }
};

/**
 * @brief Set a field value
 *
 * @tparam T Value type to set
 * @param field Field name to set
 * @param value Value to set the field to
 *
 * Explicit template instantiations are provided for std::string and ID only.
 */
template<typename T>
void set(const Object& obj, const std::string& field, const T& value) {
  using namespace bsoncxx::builder::stream;

  const auto collection = obj.collection();
  const auto& id = obj.id();

  auto selection = document{} << "_id" << id.bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << field << Serialization<T>::serialize(value)
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  auto options = mongocxx::options::find_one_and_update();
  options.projection(document{} << "_id" << 1 << finalize);
  collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
}

/**
 * @brief Get a field value, if present
 *
 * @tparam T Value type to fetch.
 * @param field Field name to set
 * @param value Value to set the field to
 */
template<typename T>
boost::optional<T> partialGet(const Object& obj, const std::string& field) {
  using namespace bsoncxx::builder::stream;

  const auto collection = obj.collection();
  const ID& id = obj.id();

  auto selection = document{} << "_id" << id.bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << field << 1 << finalize);
  auto optional = collection->mongocxx().find_one(selection.view(), options);
  if (!optional) {
    return boost::none;
  }
  return Serialization<T>::deserialize(optional.value().view()[field]);
}

template<typename T>
T get(const Object& obj, const std::string& field) {
  if (auto maybeValue = partialGet<T>(obj, field)) {
    return *maybeValue;
  }

  throw Exceptions::MissingIdOrField();
}

//! Remove a field from the database representation
inline void unset(const Object& obj, const std::string& field) {
  using namespace bsoncxx::builder::stream;

  auto collection = obj.collection();
  const ID& id = obj.id();
  auto selection = document{} << "_id" << id.bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$unset" << open_document
                             << field << ""
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  auto options = mongocxx::options::find_one_and_update();
  options.projection(document{} << "_id" << 1 << finalize);
  collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
}

//! Check whether a field exists
inline bool exists(const Object& obj, const std::string& field) {
  using namespace bsoncxx::builder::stream;

  auto collection = obj.collection();
  const ID& id = obj.id();
  /* selection:
   *   { _id : <ID> }
   */
  auto selection = document{} << "_id" << id.bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << field << 1 << finalize);
  auto optional = collection->mongocxx().find_one(selection.view(), options);
  const auto view = optional.value().view();
  return view.find(field) != view.cend();
}

//! Checks whether a field is non-empty
inline bool nonNull(const Object& obj, const std::string& field) {
  using namespace bsoncxx::builder::stream;

  auto collection = obj.collection();
  const ID& id = obj.id();
  /* selection:
   * { $and : [
   *   { _id : <ID> },
   *   { "runtime" : { $exists : true, $ne: null  } }
   *  ]}
   *  TODO: This query is very slow but I have not looked up a better alternative yet.
   */
  // clang-format off
  auto selection = document{} << "$and" << open_array
                              << open_document << "_id" << id.bsoncxx() << close_document
                              << open_document << field << open_document
                                << "$exists" << "true"
                                << "$ne" << ""
                                << close_document << close_document
                              << close_array
                              << finalize;
  // clang-format on
  auto options = mongocxx::options::find();
  options.projection(document{} << "_id" << 1 << finalize);
  auto optional = collection->mongocxx().find_one(selection.view(), options);
  return static_cast<bool>(optional);
}

template<typename T, typename R>
inline R getIntegerFromElement(T v) {
  if (v.type() == bsoncxx::type::k_int64) {
    return static_cast<R>(v.get_int64());
  }
  if (v.type() == bsoncxx::type::k_int32) {
    return static_cast<R>(v.get_int32());
  }
  if (v.type() == bsoncxx::type::k_double) {
    std::cerr << "Warning: The database contains a double value for an integer field." << std::endl;
    return static_cast<R>(v.get_double());
  }
  throw std::runtime_error("The database contains a non-integer value for an integer field.");
}

template<typename R>
inline R getInteger(bsoncxx::document::view& view, const std::string& key) {
  return getIntegerFromElement<bsoncxx::document::element, R>(view[key]);
}

// define template of getInteger explicitly for int return type to include check for overflow
template<>
inline int getInteger<int>(bsoncxx::document::view& view, const std::string& key) {
  const long x = getInteger<long>(view, key);
  if (x > std::numeric_limits<int>::max() || x < std::numeric_limits<int>::min()) {
    throw std::runtime_error("The database contains an 64bit integer that cannot be represented as 32bit integer "
                             "as defined in our database schema.");
  }
  return getIntegerFromElement<bsoncxx::document::element, int>(view[key]);
}

} /* namespace Fields */
} /* namespace Database */
} /* namespace Scine */

#endif
