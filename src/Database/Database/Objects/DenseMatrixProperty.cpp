/**
 * @file DenseMatrixProperty.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Include */
#include "Database/Objects/Impl/DerivedProperty.h"
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Id.h>
#include <Database/Objects/DenseMatrixProperty.h>

namespace Scine {
namespace Database {

constexpr const char* DenseMatrixProperty::objecttype;
constexpr const char* DenseMatrixProperty::propertytype;

DenseMatrixProperty DenseMatrixProperty::create(const std::string& name, const Model& model,
                                                const Eigen::MatrixXd& data, const CollectionPtr& collection) {
  return DerivedProperty::create<DenseMatrixProperty>(collection, model, name, data, boost::none, boost::none);
}

DenseMatrixProperty DenseMatrixProperty::create(const std::string& name, const Model& model, const Eigen::MatrixXd& data,
                                                const ID& structure, const ID& calculation, const CollectionPtr& collection) {
  return DerivedProperty::create<DenseMatrixProperty>(collection, model, name, data, structure, calculation);
}

ID DenseMatrixProperty::create(const Model& model, const std::string& propertyName, const Eigen::MatrixXd& data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  auto property =
      DerivedProperty::create<DenseMatrixProperty>(_collection, model, propertyName, data, boost::none, boost::none);
  this->_id = std::make_unique<ID>(property.id());
  return *(this->_id);
}

ID DenseMatrixProperty::create(const Model& model, const std::string& propertyName, const ID& structure,
                               const ID& calculation, const Eigen::MatrixXd& data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  auto property = DerivedProperty::create<DenseMatrixProperty>(_collection, model, propertyName, data, structure, calculation);
  this->_id = std::make_unique<ID>(property.id());
  return *this->_id;
}

Eigen::MatrixXd DenseMatrixProperty::getData() const {
  return DerivedProperty::getData<Eigen::MatrixXd>(*this);
}

void DenseMatrixProperty::setData(const Eigen::MatrixXd& data) const {
  return DerivedProperty::updateData(*this, data);
}

} /* namespace Database */
} /* namespace Scine */
