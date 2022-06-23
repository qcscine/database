/**
 * @file VectorProperty.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Include */
#include "Database/Objects/VectorProperty.h"
#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Objects/Impl/DerivedProperty.h"
#include "Database/Objects/Model.h"
/* External Includes */
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/collection.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

namespace Scine {
namespace Database {

constexpr const char* VectorProperty::objecttype;
constexpr const char* VectorProperty::propertytype;

VectorProperty VectorProperty::create(const std::string& name, const Model& model, const Eigen::VectorXd& data,
                                      const CollectionPtr& collection) {
  return DerivedProperty::create<VectorProperty>(collection, model, name, data, boost::none, boost::none);
}

VectorProperty VectorProperty::create(const std::string& name, const Model& model, const Eigen::VectorXd& data,
                                      const ID& structure, const ID& calculation, const CollectionPtr& collection) {
  return DerivedProperty::create<VectorProperty>(collection, model, name, data, structure, calculation);
}

ID VectorProperty::create(const Model& model, const std::string& propertyName, const Eigen::VectorXd& data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  auto property = DerivedProperty::create<VectorProperty>(_collection, model, propertyName, data, boost::none, boost::none);
  this->_id = std::make_unique<ID>(property.id());
  return *this->_id;
}

ID VectorProperty::create(const Model& model, const std::string& propertyName, const ID& structure,
                          const ID& calculation, const Eigen::VectorXd& data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  auto property = DerivedProperty::create<VectorProperty>(_collection, model, propertyName, data, structure, calculation);
  this->_id = std::make_unique<ID>(property.id());
  return *this->_id;
}

Eigen::VectorXd VectorProperty::getData() const {
  return DerivedProperty::getData<Eigen::VectorXd>(*this);
}

void VectorProperty::setData(const Eigen::VectorXd& data) const {
  DerivedProperty::updateData(*this, data);
}

} /* namespace Database */
} // namespace Scine
