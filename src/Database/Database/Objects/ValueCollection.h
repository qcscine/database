/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef INCLUDE_SCINE_DATABASE_VALUE_COLLECTION_H
#define INCLUDE_SCINE_DATABASE_VALUE_COLLECTION_H

#include "Database/Objects/Object.h"
#include "Utils/UniversalSettings/ValueCollection.h"
#include "boost/optional.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/types.hpp>

namespace Scine {
namespace Database {
namespace Serialization {

struct GenericValue {
  /* NOTE: This is genuinely necessary since there's been an API break between
   * mongocxx 3.4.0 and 3.6.0 changing the return type of the function below,
   * so we're just going to get the type from the compiler.
   */
  using BsonValueType = decltype(std::declval<bsoncxx::document::element>().get_value());

  static void serialize(bsoncxx::builder::basic::document& document, const std::string& key,
                        const Utils::UniversalSettings::GenericValue& value);
  static Utils::UniversalSettings::GenericValue deserialize(const BsonValueType& value);
};

struct ValueCollection {
  static bsoncxx::v_noabi::document::value serialize(const Utils::UniversalSettings::ValueCollection& collection);
  static Utils::UniversalSettings::ValueCollection deserialize(const bsoncxx::v_noabi::document::view& document);
};

} // namespace Serialization
} // namespace Database
} // namespace Scine

#endif
