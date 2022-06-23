/**
 * @file StringProperty.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include "Database/Objects/Impl/DerivedProperty.h"
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/StringProperty.h>

namespace Scine {
namespace Database {

constexpr const char* StringProperty::objecttype;
constexpr const char* StringProperty::propertytype;

StringProperty StringProperty::create(const std::string& name, const Model& model, const std::string& data,
                                      const CollectionPtr& collection) {
  return DerivedProperty::create<StringProperty>(collection, model, name, data, boost::none, boost::none);
}

StringProperty StringProperty::create(const std::string& name, const Model& model, const std::string& data,
                                      const ID& structure, const ID& calculation, const CollectionPtr& collection) {
  return DerivedProperty::create<StringProperty>(collection, model, name, data, structure, calculation);
}

ID StringProperty::create(const Model& model, const std::string& propertyName, const std::string& data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  auto property = DerivedProperty::create<StringProperty>(_collection, model, propertyName, data, boost::none, boost::none);
  this->_id = std::make_unique<ID>(property.id());
  return *this->_id;
}

ID StringProperty::create(const Model& model, const std::string& propertyName, const ID& structure,
                          const ID& calculation, const std::string& data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  auto property = DerivedProperty::create<StringProperty>(_collection, model, propertyName, data, structure, calculation);
  this->_id = std::make_unique<ID>(property.id());
  return *this->_id;
}

std::string StringProperty::getData() const {
  return DerivedProperty::getData<std::string>(*this);
}

void StringProperty::setData(const std::string& data) const {
  DerivedProperty::updateData(*this, data);
}

} /* namespace Database */
} /* namespace Scine */
