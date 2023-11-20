/**
 * @file SparseMatrixProperty.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Include */
#include "Database/Objects/Impl/DerivedProperty.h"
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/SparseMatrixProperty.h>
/* External Includes */
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/collection.hpp>

using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

namespace Scine {
namespace Database {

constexpr const char* SparseMatrixProperty::objecttype;
constexpr const char* SparseMatrixProperty::propertytype;

SparseMatrixProperty SparseMatrixProperty::create(const std::string& name, const Model& model,
                                                  const Eigen::SparseMatrix<double>& data, const CollectionPtr& collection) {
  return DerivedProperty::create<SparseMatrixProperty>(collection, model, name, data, boost::none, boost::none);
}

SparseMatrixProperty SparseMatrixProperty::create(const std::string& name, const Model& model,
                                                  const Eigen::SparseMatrix<double>& data, const ID& structure,
                                                  const ID& calculation, const CollectionPtr& collection) {
  return DerivedProperty::create<SparseMatrixProperty>(collection, model, name, data, structure, calculation);
}

ID SparseMatrixProperty::create(const Model& model, const std::string& propertyName, const Eigen::SparseMatrix<double>& data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  auto property =
      DerivedProperty::create<SparseMatrixProperty>(_collection, model, propertyName, data, boost::none, boost::none);
  this->_id = std::make_unique<ID>(property.id());
  return *(this->_id);
}

ID SparseMatrixProperty::create(const Model& model, const std::string& propertyName, const ID& structure,
                                const ID& calculation, const Eigen::SparseMatrix<double>& data) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  auto property =
      DerivedProperty::create<SparseMatrixProperty>(_collection, model, propertyName, data, structure, calculation);
  this->_id = std::make_unique<ID>(property.id());
  return *(this->_id);
}

Eigen::SparseMatrix<double> SparseMatrixProperty::getData() const {
  return DerivedProperty::getData<Eigen::SparseMatrix<double>>(*this);
}

void SparseMatrixProperty::setData(const Eigen::SparseMatrix<double>& data) const {
  DerivedProperty::updateData(*this, data);
}

} /* namespace Database */
} /* namespace Scine */
