/**
 * @file NumberProperty.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
/* Internal Include */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Objects/Impl/DerivedProperty.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/NumberProperty.h>

namespace Scine {
namespace Database {

constexpr const char* NumberProperty::objecttype;
constexpr const char* NumberProperty::propertytype;

NumberProperty NumberProperty::create(const std::string& name, const Model& model, const double data,
                                      const CollectionPtr& collection) {
  return DerivedProperty::create<NumberProperty>(collection, model, name, data, boost::none, boost::none);
}

NumberProperty NumberProperty::create(const std::string& name, const Model& model, const double data,
                                      const ID& structure, const ID& calculation, const CollectionPtr& collection) {
  return DerivedProperty::create<NumberProperty>(collection, model, name, data, structure, calculation);
}

ID NumberProperty::create(const Model& model, const std::string& propertyName, const double data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }
  auto property = DerivedProperty::create<NumberProperty>(_collection, model, propertyName, data, boost::none, boost::none);
  this->_id = std::make_unique<ID>(property.id());
  return *this->_id;
}

ID NumberProperty::create(const Model& model, const std::string& propertyName, const ID& structure,
                          const ID& calculation, const double data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }
  auto property = DerivedProperty::create<NumberProperty>(_collection, model, propertyName, data, structure, calculation);
  this->_id = std::make_unique<ID>(property.id());
  return *(this->_id);
}

double NumberProperty::getData() const {
  return DerivedProperty::getData<double>(*this);
}

void NumberProperty::setData(const double data) const {
  return DerivedProperty::updateData(*this, data);
}

} /* namespace Database */
} /* namespace Scine */
