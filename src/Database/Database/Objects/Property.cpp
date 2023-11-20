/**
 * @file Property.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
/* Internal Include */
#include "Database/Objects/Property.h"
#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Manager.h"
#include "Database/Objects/BoolProperty.h"
#include "Database/Objects/Calculation.h"
#include "Database/Objects/DenseMatrixProperty.h"
#include "Database/Objects/Impl/Fields.h"
#include "Database/Objects/Model.h"
#include "Database/Objects/NumberProperty.h"
#include "Database/Objects/SparseMatrixProperty.h"
#include "Database/Objects/StringProperty.h"
#include "Database/Objects/Structure.h"
#include "Database/Objects/VectorProperty.h"
/* External Includes */
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/collection.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

namespace Scine {
namespace Database {

constexpr const char* Property::objecttype;

template<class PropertyClass>
PropertyClass Property::getDerived() const {
  if (!this->isOfType<PropertyClass>())
    throw Exceptions::ObjectTypeMismatchException();
  PropertyClass retval(this->id().bsoncxx());
  if (this->hasLink())
    retval.link(this->collection());
  return retval;
}

template<class PropertyClass>
bool Property::isOfType() const {
  if (!this->_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto type = Fields::get<std::string>(*this, "_propertytype");
  return type == (std::string)PropertyClass::propertytype;
}

std::string Property::getPropertyName() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "property_name" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  if (view["property_name"].type() != bsoncxx::types::b_utf8::type_id)
    throw Exceptions::MissingIdOrField();
  return view["property_name"].get_utf8().value.to_string();
}

void Property::setPropertyName(const std::string& name) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "property_name" << name
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

Model Property::getModel() const {
  return Fields::get<Model>(*this, "model");
}

void Property::setModel(const Model& model) const {
  Fields::set(*this, "model", model);
}

ID Property::getStructure() const {
  return Fields::get<ID>(*this, "structure");
}

Structure Property::getStructure(const Manager& manager, const std::string& collection) const {
  return manager.getCollection(collection)->get<Structure>(getStructure());
}

boost::optional<ID> Property::partialGetStructureId() const {
  return Fields::partialGet<ID>(*this, "structure");
}

bool Property::hasStructure() const {
  return Fields::exists(*this, "structure");
}

void Property::setStructure(const ID& id) const {
  Fields::set(*this, "structure", id);
}

void Property::clearStructure() const {
  Fields::unset(*this, "structure");
}

ID Property::getCalculation() const {
  return Fields::get<ID>(*this, "calculation");
}

Calculation Property::getCalculation(const Manager& manager, const std::string& collection) const {
  return manager.getCollection(collection)->get<Calculation>(getCalculation());
}

bool Property::hasCalculation() const {
  return Fields::exists(*this, "calculation");
}

void Property::setCalculation(const ID& id) const {
  Fields::set(*this, "calculation", id);
}

void Property::clearCalculation() const {
  Fields::unset(*this, "calculation");
}

std::string Property::getComment() const {
  return Fields::get<std::string>(*this, "comment");
}

void Property::setComment(const std::string& comment) const {
  Fields::set(*this, "comment", comment);
}

bool Property::hasComment() const {
  return Fields::nonNull(*this, "comment");
}

void Property::clearComment() const {
  this->setComment("");
}

template NumberProperty Property::getDerived<NumberProperty>() const;
template DenseMatrixProperty Property::getDerived<DenseMatrixProperty>() const;
template SparseMatrixProperty Property::getDerived<SparseMatrixProperty>() const;
template VectorProperty Property::getDerived<VectorProperty>() const;
template BoolProperty Property::getDerived<BoolProperty>() const;
template StringProperty Property::getDerived<StringProperty>() const;

template bool Property::isOfType<NumberProperty>() const;
template bool Property::isOfType<VectorProperty>() const;
template bool Property::isOfType<DenseMatrixProperty>() const;
template bool Property::isOfType<SparseMatrixProperty>() const;
template bool Property::isOfType<BoolProperty>() const;
template bool Property::isOfType<StringProperty>() const;

} /* namespace Database */
} /* namespace Scine */
