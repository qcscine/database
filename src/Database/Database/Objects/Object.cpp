/**
 * @file Object.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Includes*/
#include "Database/Objects/Object.h"
#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Objects/Impl/Fields.h"
/* External Includes */
#include "boost/optional.hpp"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <cassert>
#include <iostream>
#include <memory>
#include <mongocxx/collection.hpp>

using bsoncxx::to_json;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

namespace Scine {
namespace Database {

Object::Object() : _id(nullptr) {
}

Object::Object(const ID& id) : _id(std::make_unique<ID>(id)) {
}

Object::Object(const ID& id, CollectionPtr collection)
  : _collection(std::move(collection)), _id(std::make_unique<ID>(id)) {
}

Object& Object::operator=(const Object& obj) {
  if (obj.hasId()) {
    this->_id = std::make_unique<ID>(obj.id());
  }
  else {
    this->_id.reset(nullptr);
  }
  if (obj._collection) {
    this->_collection = obj._collection;
  }
  else {
    this->_collection = nullptr;
  }
  return *this;
}

void Object::link(std::shared_ptr<Collection> collection) {
  _collection = std::move(collection);
}
void Object::detach() {
  _collection = nullptr;
}
bool Object::hasLink() const {
  return _collection != nullptr;
}

Object::Object(const Object& other) {
  if (other.hasId()) {
    this->_id = std::make_unique<ID>(other.id());
  }
  else {
    this->_id.reset(nullptr);
  }
  if (other._collection) {
    this->_collection = other._collection;
  }
  else {
    this->_collection = nullptr;
  }
}

bsoncxx::document::value Object::getRawContent() const {
  if (!this->_id)
    throw Exceptions::MissingIDException();
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  auto optional = _collection->mongocxx().find_one(selection.view());
  if (!optional)
    throw Exceptions::IDNotFoundException();
  return optional.value();
}

std::string Object::json() const {
  return to_json(this->getRawContent().view());
  ;
}

void Object::print() const {
  std::cout << this->json() << std::endl;
}

const ID& Object::id() const {
  if (!this->hasId())
    throw Exceptions::MissingIDException();
  return *_id;
}

bool Object::exists() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  return _collection->has(this->id().bsoncxx());
}

std::shared_ptr<Collection> Object::collection() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  return _collection;
}

void Object::wipe(bool expectPresence) {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  if (expectPresence) {
    auto optional = _collection->mongocxx().find_one(selection.view());
    if (!optional)
      throw Exceptions::IDNotFoundException();
  }
  _collection->mongocxx().delete_one(selection.view());
  this->_id = nullptr;
}

std::chrono::system_clock::time_point Object::created() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "_created" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::IDNotFoundException();
  return {optional.value().view()["_created"].get_date()};
}

std::chrono::system_clock::time_point Object::lastModified() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "_lastmodified" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::IDNotFoundException();
  return {optional.value().view()["_lastmodified"].get_date()};
}

bool Object::hasCreatedTimestamp() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  return Fields::exists(*this, "_created");
}

bool Object::hasLastModifiedTimestamp() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  return Fields::exists(*this, "_lastmodified");
}

bool Object::olderThan(const Object& other, bool modification) const {
  if (modification) {
    return this->lastModified() < other.lastModified();
  }

  return this->created() < other.created();
}

void Object::touch() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Object::enable_analysis() {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "analysis_disabled" << false
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Object::enable_exploration() {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "exploration_disabled" << false
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Object::disable_analysis() {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "analysis_disabled" << true
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Object::disable_exploration() {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "exploration_disabled" << true
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

bool Object::analyze() {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  return !Fields::get<bool>(*this, "analysis_disabled");
}

bool Object::explore() {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  return !Fields::get<bool>(*this, "exploration_disabled");
}

} /* namespace Database */
} /* namespace Scine */
