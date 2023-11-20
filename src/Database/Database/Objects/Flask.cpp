/**
 * @file Flask.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Includes*/
#include "Database/Objects/Flask.h"
#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Manager.h"
#include "Database/Objects/Compound.h"
#include "Database/Objects/Impl/Fields.h"
#include "Database/Objects/Reaction.h"
#include "Database/Objects/Structure.h"
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
namespace {

ID createImpl(const std::vector<ID>& structures, const std::vector<ID>& compounds, const Flask::CollectionPtr& collection) {
  // Build arrays
  bsoncxx::builder::basic::array structureArray;
  for (const auto& id : structures) {
    structureArray.append(id.bsoncxx());
  }
  bsoncxx::builder::basic::array compoundArray;
  for (const auto& id : compounds) {
    compoundArray.append(id.bsoncxx());
  }

  auto now = bsoncxx::types::b_date(std::chrono::system_clock::now());
  // clang-format off
  auto doc = document{} << "_created" << now
                        << "_lastmodified" << now
                        << "analysis_disabled" << false
                        << "exploration_disabled" << false
                        << "_objecttype" << Flask::objecttype
                        << "structures" << structureArray
                        << "compounds" << compoundArray
                        << "reactions" << open_array << close_array
                        << finalize;
  // clang-format on
  auto result = collection->mongocxx().insert_one(doc.view());
  return {result->inserted_id().get_oid().value};
}

} // namespace

constexpr const char* Flask::objecttype;

Flask Flask::create(const std::vector<ID>& structures, const std::vector<ID>& compounds, const CollectionPtr& collection) {
  if (!collection) {
    throw Exceptions::MissingCollectionException();
  }

  return Flask{createImpl(structures, compounds, collection), collection};
}

ID Flask::create(const std::vector<ID>& structures, const std::vector<ID>& compounds) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  this->_id = std::make_unique<ID>(createImpl(structures, compounds, _collection));
  return *this->_id;
}

ID Flask::getCentroid() const {
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

Structure Flask::getCentroid(const Manager& manager, const std::string& collection) const {
  return manager.getCollection(collection)->get<Structure>(getCentroid());
}

bool Flask::hasReaction(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  /* selection:
   * { $and : [
   *   { _id : <ID> },
   *   { "reactions" : { $elemMatch : { $eq : <id> }}} }
   *  ]}
   */
  // clang-format off
  auto selection = document{} << "$and" << open_array
                              << open_document << "_id" << this->id().bsoncxx() << close_document
                              << open_document << "reactions" << open_document
                                << "$elemMatch" << open_document << "$eq" << id.bsoncxx()
                                << close_document << close_document << close_document
                              << close_array
                              << finalize;
  // clang-format on
  auto optional = _collection->mongocxx().find_one(selection.view());
  return static_cast<bool>(optional);
}

void Flask::addReaction(const ID& id) const {
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

void Flask::removeReaction(const ID& id) const {
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

int Flask::hasReactions() const {
  return Fields::get<std::vector<ID>>(*this, "reactions").size();
}

std::vector<ID> Flask::getReactions() const {
  return Fields::get<std::vector<ID>>(*this, "reactions");
}

std::vector<Reaction> Flask::getReactions(const Manager& manager, const std::string& collection) const {
  auto ids = getReactions();
  std::vector<Reaction> reactions;
  auto reactionCollection = manager.getCollection(collection);
  reactions.reserve(ids.size());
  for (auto& id : ids) {
    reactions.emplace_back(std::move(id), reactionCollection);
  }
  return reactions;
}

void Flask::setReactions(const std::vector<ID>& ids) const {
  Fields::set(*this, "reactions", ids);
}

void Flask::clearReactions() const {
  setReactions({});
}

bool Flask::hasStructure(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  /* selection:
   * { $and : [
   *   { _id : <ID> },
   *   { "structures" : { $elemMatch : { $eq : <id> }}} }
   *  ]}
   */
  // clang-format off
  auto selection = document{} << "$and" << open_array
                              << open_document << "_id" << this->id().bsoncxx() << close_document
                              << open_document << "structures" << open_document
                                << "$elemMatch" << open_document << "$eq" << id.bsoncxx()
                                << close_document << close_document << close_document
                              << close_array
                              << finalize;
  // clang-format on
  auto optional = _collection->mongocxx().find_one(selection.view());
  return static_cast<bool>(optional);
}

void Flask::addStructure(const ID& id) const {
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

void Flask::removeStructure(const ID& id) const {
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

int Flask::hasStructures() const {
  return Fields::get<std::vector<ID>>(*this, "structures").size();
}

std::vector<ID> Flask::getStructures() const {
  return Fields::get<std::vector<ID>>(*this, "structures");
}

std::vector<Structure> Flask::getStructures(const Manager& manager, const std::string& collection) const {
  auto ids = getStructures();
  std::vector<Structure> structures;
  auto reactionCollection = manager.getCollection(collection);
  structures.reserve(ids.size());
  for (auto& id : ids) {
    structures.emplace_back(std::move(id), reactionCollection);
  }
  return structures;
}

void Flask::setStructures(const std::vector<ID>& ids) const {
  Fields::set(*this, "structures", ids);
}

void Flask::clearStructures() const {
  setStructures({});
}

bool Flask::hasCompound(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  /* selection:
   * { $and : [
   *   { _id : <ID> },
   *   { "compounds" : { $elemMatch : { $eq : <id> }}} }
   *  ]}
   */
  // clang-format off
  auto selection = document{} << "$and" << open_array
                              << open_document << "_id" << this->id().bsoncxx() << close_document
                              << open_document << "compounds" << open_document
                                << "$elemMatch" << open_document << "$eq" << id.bsoncxx()
                                << close_document << close_document << close_document
                              << close_array
                              << finalize;
  // clang-format on
  auto optional = _collection->mongocxx().find_one(selection.view());
  return static_cast<bool>(optional);
}

int Flask::hasCompounds() const {
  return Fields::get<std::vector<ID>>(*this, "compounds").size();
}

std::vector<Compound> Flask::getCompounds(const Manager& manager, const std::string& collection) const {
  auto ids = getCompounds();
  std::vector<Compound> compounds;
  auto compoundCollection = manager.getCollection(collection);
  compounds.reserve(ids.size());
  for (auto& id : ids) {
    compounds.emplace_back(std::move(id), compoundCollection);
  }
  return compounds;
}

std::vector<ID> Flask::getCompounds() const {
  return Fields::get<std::vector<ID>>(*this, "compounds");
}

void Flask::setCompounds(const std::vector<ID>& ids) const {
  Fields::set(*this, "compounds", ids);
}

void Flask::clearCompounds() const {
  setCompounds({});
}

} /* namespace Database */
} /* namespace Scine */
