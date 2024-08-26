/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include "Database/Objects/ValueCollection.h"
#include "Database/Exceptions.h"
#include "Database/Objects/Impl/Fields.h"
#include "Utils/UniversalSettings/GenericValueVariant.h"
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/types/value.hpp>
#include <tuple>

namespace Scine {
namespace Database {
namespace Serialization {

namespace {

void anon_serialize(bsoncxx::builder::basic::document& document, const std::string& key,
                    const Utils::UniversalSettings::ValueCollection& collection) {
  using bsoncxx::builder::basic::kvp;
  document.append(kvp(key, ValueCollection::serialize(collection)));
}
void anon_serialize(bsoncxx::builder::basic::document& document, const std::string& key, bool v) {
  using bsoncxx::builder::basic::kvp;
  document.append(kvp(key, v));
}
void anon_serialize(bsoncxx::builder::basic::document& document, const std::string& key, int v) {
  using bsoncxx::builder::basic::kvp;
  document.append(kvp(key, v));
}
void anon_serialize(bsoncxx::builder::basic::document& document, const std::string& key, double v) {
  using bsoncxx::builder::basic::kvp;
  document.append(kvp(key, v));
}
void anon_serialize(bsoncxx::builder::basic::document& document, const std::string& key, const std::string& v) {
  using bsoncxx::builder::basic::kvp;
  document.append(kvp(key, v));
}
void anon_serialize(bsoncxx::builder::basic::document& document, const std::string& key,
                    const Utils::UniversalSettings::ParametrizedOptionValue& v) {
  bsoncxx::builder::basic::document builder{};
  using bsoncxx::builder::basic::kvp;

  anon_serialize(builder, "selectedOption", v.selectedOption);
  anon_serialize(builder, "optionSettings", v.optionSettings);

  document.append(kvp(key, builder.extract()));
}

void array_append(bsoncxx::builder::basic::array& array, int v) {
  array.append(v);
}
void array_append(bsoncxx::builder::basic::array& array, double v) {
  array.append(v);
}
void array_append(bsoncxx::builder::basic::array& array, const std::string& v) {
  array.append(v);
}
void array_append(bsoncxx::builder::basic::array& array, const Utils::UniversalSettings::ValueCollection& v) {
  array.append(ValueCollection::serialize(v));
}
void array_append(bsoncxx::builder::basic::array& array, const std::vector<int>& v) {
  /*
   * MongoDB does not support arrays of arrays. Therefore, we construct an array of value collections.
   * These value collections then contain exactly one array which is the inner array.
   */
  Utils::UniversalSettings::ValueCollection newValueCollection;
  newValueCollection.addIntList(subListKey, v);
  array.append(ValueCollection::serialize(newValueCollection));
}

// Blanket impl for IntList, DoubleList, StringList and CollectionList
template<typename T>
void serialize_array(bsoncxx::builder::basic::document& document, const std::string& key, const std::vector<T>& v) {
  bsoncxx::builder::basic::array builder{};
  for (const auto& value : v) {
    array_append(builder, value);
  }
  using bsoncxx::builder::basic::kvp;
  document.append(kvp(key, builder.extract()));
}

template<typename T>
struct ListTypeHint {};

template<>
struct ListTypeHint<int> {
  static std::string value() {
    return "int";
  }
};
template<>
struct ListTypeHint<double> {
  static std::string value() {
    return "double";
  }
};
template<>
struct ListTypeHint<std::string> {
  static std::string value() {
    return "str";
  }
};
template<>
struct ListTypeHint<Utils::UniversalSettings::ValueCollection> {
  static std::string value() {
    return "value_coll";
  }
};

template<>
struct ListTypeHint<std::vector<int>> {
  static std::string value() {
    return "list_of_lists";
  }
};

template<typename T>
void anon_serialize(bsoncxx::builder::basic::document& document, const std::string& key, const std::vector<T>& v) {
  bsoncxx::builder::basic::document builder{};
  using bsoncxx::builder::basic::kvp;
  builder.append(kvp("type", ListTypeHint<std::decay_t<T>>::value()));
  serialize_array(builder, "list", v);
  document.append(kvp(key, builder.extract()));
}

} // namespace

void GenericValue::serialize(bsoncxx::builder::basic::document& document, const std::string& key,
                             const Utils::UniversalSettings::GenericValue& value) {
  bool foundType = false;
  auto checkType = [&](auto&& fns) {
    if (!foundType && std::get<0>(fns)(value)) {
      anon_serialize(document, key, std::get<1>(fns)(value));
      foundType = true;
    }
  };
  std::apply([&](auto&&... fns) { ((checkType(fns)), ...); },
             Utils::UniversalSettings::GenericValueMeta::zip(Utils::UniversalSettings::GenericValueMeta::type_checkers(),
                                                             Utils::UniversalSettings::GenericValueMeta::getters()));
}

Utils::UniversalSettings::GenericValue GenericValue::deserialize(const BsonValueType& value) {
  const auto type = value.type();
  if (type == bsoncxx::type::k_bool) {
    return Utils::UniversalSettings::GenericValue::fromBool(value.get_bool());
  }
  if (type == bsoncxx::type::k_int32) {
    return Utils::UniversalSettings::GenericValue::fromInt(Fields::getIntegerFromElement<BsonValueType, int>(value));
  }
  if (type == bsoncxx::type::k_double) {
    return Utils::UniversalSettings::GenericValue::fromDouble(value.get_double());
  }
  if (type == bsoncxx::type::k_utf8) {
    return Utils::UniversalSettings::GenericValue::fromString(value.get_utf8().value.to_string());
  }
  if (type == bsoncxx::type::k_document) {
    /* Possible types:
     * - IntList, DoubleList, StringList, CollectionList (has type and list elements)
     * - ParametrizedOptionValue (has selectedOption member)
     * - ValueCollection
     */
    auto document = value.get_document();
    auto typeIterator = document.view().find("type");
    if (typeIterator != document.view().end()) {
      // Lists of things
      const std::string typeString = typeIterator->get_utf8().value.to_string();
      auto array = document.view()["list"].get_array();
      if (typeString == ListTypeHint<int>::value()) {
        std::vector<int> vs;
        for (const auto v : array.value) {
          vs.push_back(Fields::getIntegerFromElement<bsoncxx::array::element, int>(v));
        }
        return Utils::UniversalSettings::GenericValue::fromIntList(std::move(vs));
      }
      if (typeString == ListTypeHint<double>::value()) {
        std::vector<double> vs;
        for (const auto v : array.value) {
          vs.push_back(v.get_double());
        }
        return Utils::UniversalSettings::GenericValue::fromDoubleList(std::move(vs));
      }
      if (typeString == ListTypeHint<std::string>::value()) {
        std::vector<std::string> vs;
        for (const auto v : array.value) {
          vs.push_back(v.get_utf8().value.to_string());
        }
        return Utils::UniversalSettings::GenericValue::fromStringList(std::move(vs));
      }
      if (typeString == ListTypeHint<Utils::UniversalSettings::ValueCollection>::value()) {
        std::vector<Utils::UniversalSettings::ValueCollection> vs;
        for (auto v : array.value) {
          vs.push_back(ValueCollection::deserialize(v.get_document()));
        }
        return Utils::UniversalSettings::GenericValue::fromCollectionList(std::move(vs));
      }
      if (typeString == ListTypeHint<std::vector<int>>::value()) {
        std::vector<std::vector<int>> listOfLists;
        for (auto v : array.value) {
          auto collection = ValueCollection::deserialize(v.get_document());
          listOfLists.push_back(collection.getIntList(subListKey));
        }
        return Utils::UniversalSettings::GenericValue::fromIntListList(std::move(listOfLists));
      }

      throw Exceptions::MissingIdOrField();
    }
    if (document.view().find("selectedOption") != document.view().end()) {
      Utils::UniversalSettings::ParametrizedOptionValue option{
          document.view()["selectedOption"].get_utf8().value.to_string(),
          ValueCollection::deserialize(document.view()["optionSettings"].get_document())};
      return Utils::UniversalSettings::GenericValue::fromOptionWithSettings(std::move(option));
    }
    return Utils::UniversalSettings::GenericValue::fromCollection(ValueCollection::deserialize(document));
  }

  throw Exceptions::MissingIdOrField();
}

bsoncxx::v_noabi::document::value ValueCollection::serialize(const Utils::UniversalSettings::ValueCollection& collection) {
  bsoncxx::builder::basic::document builder{};
  using bsoncxx::builder::basic::kvp;

  for (const std::string& key : collection.getKeys()) {
    GenericValue::serialize(builder, key, collection.getValue(key));
  }

  return builder.extract();
}

Utils::UniversalSettings::ValueCollection ValueCollection::deserialize(const bsoncxx::v_noabi::document::view& document) {
  Utils::UniversalSettings::ValueCollection collection;

  for (const auto& element : document) {
    collection.addGenericValue(element.key().to_string(), GenericValue::deserialize(element.get_value()));
  }

  return collection;
}

} // namespace Serialization
} // namespace Database
} // namespace Scine
