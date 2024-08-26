/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Includes*/
#include "Database/Objects/Reaction.h"
#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Manager.h"
#include "Database/Objects/Compound.h"
#include "Database/Objects/ElementaryStep.h"
#include "Database/Objects/Impl/Fields.h"
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

ID createImpl(const std::vector<ID>& lhs, const std::vector<ID>& rhs, const Object::CollectionPtr& collection,
              const std::vector<COMPOUND_OR_FLASK>& lhsTypes, const std::vector<COMPOUND_OR_FLASK>& rhsTypes) {
  // Build LHS and RHS arrays
  auto lTypes = lhsTypes;
  if (lhsTypes.size() == 0) {
    lTypes.reserve(lhs.size());
    for (unsigned int i = 0; i < lhs.size(); i++) {
      lTypes.push_back(COMPOUND_OR_FLASK::COMPOUND);
    }
  }
  auto rTypes = rhsTypes;
  if (rhsTypes.size() == 0) {
    rTypes.reserve(rhs.size());
    for (unsigned int i = 0; i < rhs.size(); i++) {
      rTypes.push_back(COMPOUND_OR_FLASK::COMPOUND);
    }
  }
  if (lhs.size() != lTypes.size()) {
    throw std::runtime_error("Number of reagents and number of reagent types do not match.");
  }
  if (rhs.size() != rTypes.size()) {
    throw std::runtime_error("Number of reagents and number of reagent types do not match.");
  }
  bsoncxx::builder::basic::array lhsArray;
  for (unsigned int i = 0; i < lhs.size(); i++) {
    // clang-format off
    auto doc = document{} << "id" << lhs[i].bsoncxx()
                          << "type" << EnumMaps::reactant2str.at(lTypes[i])
                          << finalize;
    // clang-format on
    lhsArray.append(doc);
  }
  bsoncxx::builder::basic::array rhsArray;
  for (unsigned int i = 0; i < rhs.size(); i++) {
    // clang-format off
    auto doc = document{} << "id" << rhs[i].bsoncxx()
                          << "type" << EnumMaps::reactant2str.at(rTypes[i])
                          << finalize;
    // clang-format on
    rhsArray.append(doc);
  }

  auto now = bsoncxx::types::b_date(std::chrono::system_clock::now());
  // clang-format off
  auto doc = document{} << "_created" << now
                        << "_lastmodified" << now
                        << "analysis_disabled" << false
                        << "exploration_disabled" << false
                        << "_objecttype" << Reaction::objecttype
                        << "lhs" << lhsArray
                        << "rhs" << rhsArray
                        << "elementary_steps" << open_array << close_array
                        << finalize;
  // clang-format on
  auto result = collection->mongocxx().insert_one(doc.view());
  return {result->inserted_id().get_oid().value};
}

} // namespace

constexpr const char* Reaction::objecttype;

Reaction Reaction::create(const std::vector<ID>& lhs, const std::vector<ID>& rhs, const CollectionPtr& collection,
                          const std::vector<COMPOUND_OR_FLASK>& lhsTypes, const std::vector<COMPOUND_OR_FLASK>& rhsTypes) {
  if (!collection) {
    throw Exceptions::MissingCollectionException();
  }

  return {createImpl(lhs, rhs, collection, lhsTypes, rhsTypes), collection};
}

ID Reaction::create(const std::vector<ID>& lhs, const std::vector<ID>& rhs,
                    const std::vector<COMPOUND_OR_FLASK>& lhsTypes, const std::vector<COMPOUND_OR_FLASK>& rhsTypes) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  this->_id = std::make_unique<ID>(createImpl(lhs, rhs, _collection, lhsTypes, rhsTypes));
  return *(this->_id);
}

bool Reaction::hasElementaryStep(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  const auto elementarySteps = this->getElementarySteps();
  return std::find(elementarySteps.begin(), elementarySteps.end(), id) != elementarySteps.end();
}

void Reaction::addElementaryStep(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$push" << open_document
                             << "elementary_steps" << id.bsoncxx()
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  auto options = mongocxx::options::find_one_and_update();
  options.projection(document{} << "_id" << 1 << finalize);
  _collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
}

void Reaction::removeElementaryStep(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$pull" << open_document
                             << "elementary_steps" << id.bsoncxx()
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  auto options = mongocxx::options::find_one_and_update();
  options.projection(document{} << "_id" << 1 << finalize);
  _collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
}

int Reaction::hasElementarySteps() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "elementary_steps" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto array = view["elementary_steps"].get_array().value;
  return std::distance(array.begin(), array.end());
}

std::vector<ID> Reaction::getElementarySteps() const {
  return Fields::get<std::vector<ID>>(*this, "elementary_steps");
}

std::vector<ElementaryStep> Reaction::getElementarySteps(const Manager& manager, const std::string& collection) const {
  std::vector<ID> ids = getElementarySteps();
  auto collectionPtr = manager.getCollection(collection);
  std::vector<ElementaryStep> steps;
  steps.reserve(ids.size());
  for (auto& id : ids) {
    steps.emplace_back(std::move(id), collectionPtr);
  }
  return steps;
}

void Reaction::setElementarySteps(const std::vector<ID>& ids) const {
  Fields::set(*this, "elementary_steps", ids);
}

void Reaction::clearElementarySteps() const {
  setElementarySteps({});
}

SIDE Reaction::hasReactant(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  const auto reactants = this->getReactants(SIDE::BOTH);
  const auto& lhsIDs = std::get<0>(reactants);
  const auto& rhsIDs = std::get<1>(reactants);
  const bool lhs = std::find(lhsIDs.begin(), lhsIDs.end(), id) != lhsIDs.end();
  const bool rhs = std::find(rhsIDs.begin(), rhsIDs.end(), id) != rhsIDs.end();
  if (lhs && rhs) {
    return SIDE::BOTH;
  }
  if (lhs) {
    return SIDE::LHS;
  }
  if (rhs) {
    return SIDE::RHS;
  }

  return SIDE::NONE;
}

void Reaction::addReactant(const ID& id, const SIDE side, COMPOUND_OR_FLASK type) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  if (side == SIDE::BOTH || side == SIDE::LHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$push" << open_document
                               << "lhs" << open_document
                                 << "id" << id.bsoncxx()
                                 << "type" << EnumMaps::reactant2str.at(type)
                               << close_document
                             << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    auto options = mongocxx::options::find_one_and_update();
    options.projection(document{} << "_id" << 1 << finalize);
    _collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
  }
  if (side == SIDE::BOTH || side == SIDE::RHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$push" << open_document
                               << "rhs" << open_document
                                 << "id" << id.bsoncxx()
                                 << "type" << EnumMaps::reactant2str.at(type)
                               << close_document
                             << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    auto options = mongocxx::options::find_one_and_update();
    options.projection(document{} << "_id" << 1 << finalize);
    _collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
  }
}

void Reaction::removeReactant(const ID& id, const SIDE side) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  if (side == SIDE::BOTH || side == SIDE::LHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$pull" << open_document
                               << "lhs" << open_document << "id" << id.bsoncxx() << close_document
                               << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    auto options = mongocxx::options::find_one_and_update();
    options.projection(document{} << "_id" << 1 << finalize);
    _collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
  }
  if (side == SIDE::BOTH || side == SIDE::RHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$pull" << open_document
                               << "rhs" << open_document << "id" << id.bsoncxx() << close_document
                               << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    auto options = mongocxx::options::find_one_and_update();
    options.projection(document{} << "_id" << 1 << finalize);
    _collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
  }
}

void Reaction::setReactants(const std::vector<ID>& ids, const SIDE side, const std::vector<COMPOUND_OR_FLASK>& types) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto ts = types;
  if (types.size() == 0) {
    ts.reserve(ids.size());
    for (unsigned int i = 0; i < ids.size(); i++) {
      ts.push_back(COMPOUND_OR_FLASK::COMPOUND);
    }
  }
  if (ids.size() != ts.size()) {
    throw std::runtime_error("Number of reagents and number of reagent types do not match.");
  }
  bsoncxx::builder::basic::array array;
  for (unsigned int i = 0; i < ids.size(); i++) {
    // clang-format off
    auto doc = document{} << "id" << ids[i].bsoncxx()
                          << "type" << EnumMaps::reactant2str.at(ts[i])
                          << finalize;
    // clang-format on
    array.append(doc.view());
  }
  if (side == SIDE::BOTH || side == SIDE::LHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$set" << open_document
                               << "lhs" << array
                               << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    auto options = mongocxx::options::find_one_and_update();
    options.projection(document{} << "_id" << 1 << finalize);
    _collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
  }
  if (side == SIDE::BOTH || side == SIDE::RHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$set" << open_document
                               << "rhs" << array
                               << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    auto options = mongocxx::options::find_one_and_update();
    options.projection(document{} << "_id" << 1 << finalize);
    _collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
  }
}

std::tuple<std::vector<ID>, std::vector<ID>> Reaction::getReactants(const SIDE side) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  std::vector<ID> lhs;
  std::vector<ID> rhs;
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  if (side == SIDE::BOTH) {
    options.projection(document{} << "lhs" << 1 << "rhs" << 1 << finalize);
  }
  else if (side == SIDE::LHS) {
    options.projection(document{} << "lhs" << 1 << finalize);
  }
  else if (side == SIDE::RHS) {
    options.projection(document{} << "rhs" << 1 << finalize);
  }
  else {
    return {lhs, rhs};
  }
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  if (side == SIDE::BOTH || side == SIDE::LHS) {
    auto array = view["lhs"].get_array().value;
    for (auto element : array) {
      lhs.push_back(Fields::Serialization<ID>::deserialize(element["id"]).value());
    }
  }
  if (side == SIDE::BOTH || side == SIDE::RHS) {
    auto array = view["rhs"].get_array().value;
    for (auto element : array) {
      rhs.push_back(Fields::Serialization<ID>::deserialize(element["id"]).value());
    }
  }
  return {lhs, rhs};
}

std::tuple<int, int> Reaction::hasReactants() const {
  auto reactants = this->getReactants(SIDE::BOTH);
  return {std::get<0>(reactants).size(), std::get<1>(reactants).size()};
}

std::tuple<std::vector<COMPOUND_OR_FLASK>, std::vector<COMPOUND_OR_FLASK>> Reaction::getReactantTypes(SIDE side) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  std::vector<COMPOUND_OR_FLASK> lhs;
  std::vector<COMPOUND_OR_FLASK> rhs;
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  if (side == SIDE::BOTH) {
    options.projection(document{} << "lhs" << 1 << "rhs" << 1 << finalize);
  }
  else if (side == SIDE::LHS) {
    options.projection(document{} << "lhs" << 1 << finalize);
  }
  else if (side == SIDE::RHS) {
    options.projection(document{} << "rhs" << 1 << finalize);
  }
  else {
    return {lhs, rhs};
  }
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  if (side == SIDE::BOTH || side == SIDE::LHS) {
    auto array = view["lhs"].get_array().value;
    for (auto element : array) {
      const std::string i = element["type"].get_utf8().value.to_string();
      lhs.push_back(EnumMaps::str2reactant.at(i));
    }
  }
  if (side == SIDE::BOTH || side == SIDE::RHS) {
    auto array = view["rhs"].get_array().value;
    for (auto element : array) {
      const std::string i = element["type"].get_utf8().value.to_string();
      rhs.push_back(EnumMaps::str2reactant.at(i));
    }
  }
  return {lhs, rhs};
}

COMPOUND_OR_FLASK Reaction::getReactantType(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  // clang-format off
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format on
  mongocxx::options::find options{};
  options.projection(document{} << "lhs" << 1 << "rhs" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  std::string type = "";
  auto lhs = view["lhs"].get_array().value;
  for (auto element : lhs) {
    if (element["id"].get_oid().value == id.bsoncxx()) {
      type = element["type"].get_utf8().value.to_string();
      break;
    }
  }
  auto rhs = view["rhs"].get_array().value;
  for (auto element : rhs) {
    if (element["id"].get_oid().value == id.bsoncxx()) {
      type = element["type"].get_utf8().value.to_string();
      break;
    }
  }
  if (type.empty()) {
    throw Exceptions::MissingIdOrField();
  }
  return EnumMaps::str2reactant.at(type);
}

void Reaction::clearReactants(const SIDE side) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  if (side == SIDE::BOTH || side == SIDE::LHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$set" << open_document
                               << "lhs" << open_array << close_array
                               << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    auto options = mongocxx::options::find_one_and_update();
    options.projection(document{} << "_id" << 1 << finalize);
    _collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
  }
  if (side == SIDE::BOTH || side == SIDE::RHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$set" << open_document
                               << "rhs" << open_array << close_array
                               << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    auto options = mongocxx::options::find_one_and_update();
    options.projection(document{} << "_id" << 1 << finalize);
    _collection->mongocxx().find_one_and_update(selection.view(), update.view(), options);
  }
}

} /* namespace Database */
} /* namespace Scine */
