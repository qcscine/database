/**
 * @file NumberProperty.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
/* Internal Include */
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Objects/BoolProperty.h>
#include <Database/Objects/Impl/DerivedProperty.h>
#include <Database/Objects/Model.h>

namespace Scine {
namespace Database {

constexpr const char* BoolProperty::objecttype;
constexpr const char* BoolProperty::propertytype;

BoolProperty BoolProperty::create(const std::string& name, const Model& model, const bool data, const CollectionPtr& collection) {
  return DerivedProperty::create<BoolProperty>(collection, model, name, data, boost::none, boost::none);
}

BoolProperty BoolProperty::create(const std::string& name, const Model& model, const bool data, const ID& structure,
                                  const ID& calculation, const CollectionPtr& collection) {
  return DerivedProperty::create<BoolProperty>(collection, model, name, data, structure, calculation);
}

ID BoolProperty::create(const Model& model, const std::string& propertyName, const bool data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  auto property = DerivedProperty::create<BoolProperty>(_collection, model, propertyName, data, boost::none, boost::none);
  this->_id = std::make_unique<ID>(property.id());
  return *(this->_id);
}

ID BoolProperty::create(const Model& model, const std::string& propertyName, const ID& structure, const ID& calculation,
                        const bool data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  auto property = DerivedProperty::create<BoolProperty>(_collection, model, propertyName, data, structure, calculation);
  this->_id = std::make_unique<ID>(property.id());
  return *(this->_id);
}

bool BoolProperty::getData() const {
  return DerivedProperty::getData<bool>(*this);
}

void BoolProperty::setData(const bool data) const {
  DerivedProperty::updateData(*this, data);
}

} /* namespace Database */
} /* namespace Scine */
