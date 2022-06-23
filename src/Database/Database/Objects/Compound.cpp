/**
 * @file Compound.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Includes*/
#include "Database/Objects/Compound.h"
#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Manager.h"
#include "Database/Objects/Impl/Fields.h"
#include "Database/Objects/Reaction.h"
#include "Database/Objects/Structure.h"
/* External Includes */
#include <bsoncxx/builder/stream/document.hpp>
#include <iostream>
#include <mongocxx/collection.hpp>

using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

namespace Scine {
namespace Database {
namespace {

ID createImpl(const std::vector<ID>& structures, const Compound::CollectionPtr& collection) {
  // Build atom array
  bsoncxx::builder::basic::array structureArray;
  for (const auto& id : structures) {
    structureArray.append(id.bsoncxx());
  }

  auto now = bsoncxx::types::b_date(std::chrono::system_clock::now());
  // clang-format off
  auto doc = document{} << "_created" << now
                        << "_lastmodified" << now
                        << "analysis_disabled" << false
                        << "exploration_disabled" << false
                        << "_objecttype" << Compound::objecttype
                        << "structures" << structureArray
                        << "reactions" << open_array << close_array
                        << finalize;
  // clang-format on
  auto result = collection->mongocxx().insert_one(doc.view());
  return {result->inserted_id().get_oid().value};
}

} // namespace

constexpr const char* Compound::objecttype;

Compound Compound::create(const std::vector<ID>& structures, const CollectionPtr& collection) {
  if (!collection) {
    throw Exceptions::MissingCollectionException();
  }

  return Compound{createImpl(structures, collection), collection};
}

ID Compound::create(const std::vector<ID>& structures) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  this->_id = std::make_unique<ID>(createImpl(structures, _collection));
  return *this->_id;
}

ID Compound::getCentroid() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "structures" << open_document << "$slice" << 1 << close_document << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto array = view["structures"].get_array().value;
  if (array.empty())
    throw Exceptions::MissingIdOrField();
  return array[0].get_oid().value;
}

Structure Compound::getCentroid(const Manager& manager, const std::string& collection) const {
  return manager.getCollection(collection)->get<Structure>(getCentroid());
}

bool Compound::hasReaction(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  const auto reactions = this->getReactions();
  return std::find(reactions.begin(), reactions.end(), id) != reactions.end();
}

void Compound::addReaction(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$push" << open_document
                             << "reactions" << id.bsoncxx()
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Compound::removeReaction(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$pull" << open_document
                             << "reactions" << id.bsoncxx()
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

int Compound::hasReactions() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "reactions" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto array = view["reactions"].get_array().value;
  return std::distance(array.begin(), array.end());
}

std::vector<ID> Compound::getReactions() const {
  return Fields::get<std::vector<ID>>(*this, "reactions");
}

std::vector<Reaction> Compound::getReactions(const Manager& manager, const std::string& collection) const {
  auto ids = getReactions();
  std::vector<Reaction> reactions;
  auto reactionCollection = manager.getCollection(collection);
  reactions.reserve(ids.size());
  for (auto& id : ids) {
    reactions.emplace_back(std::move(id), reactionCollection);
  }
  return reactions;
}

void Compound::setReactions(const std::vector<ID>& ids) const {
  Fields::set(*this, "reactions", ids);
}

void Compound::clearReactions() const {
  setReactions({});
}

bool Compound::hasStructure(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  const auto structures = this->getStructures();
  return std::find(structures.begin(), structures.end(), id) != structures.end();
}

void Compound::addStructure(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$push" << open_document
                             << "structures" << id.bsoncxx()
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Compound::removeStructure(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$pull" << open_document
                             << "structures" << id.bsoncxx()
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

int Compound::hasStructures() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "structures" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto array = view["structures"].get_array().value;
  return std::distance(array.begin(), array.end());
}

std::vector<ID> Compound::getStructures() const {
  return Fields::get<std::vector<ID>>(*this, "structures");
}

std::vector<Structure> Compound::getStructures(const Manager& manager, const std::string& collection) const {
  auto ids = getStructures();
  std::vector<Structure> structures;
  auto structureCollection = manager.getCollection(collection);
  structures.reserve(ids.size());
  for (auto& id : ids) {
    structures.emplace_back(std::move(id), structureCollection);
  }
  return structures;
}

void Compound::setStructures(const std::vector<ID>& ids) const {
  Fields::set(*this, "structures", ids);
}

void Compound::clearStructures() const {
  setStructures({});
}

} /* namespace Database */
} /* namespace Scine */
