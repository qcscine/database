/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
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
#include <Database/Objects/ReactionSide.h>
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

ID createImpl(const std::vector<ID>& lhs, const std::vector<ID>& rhs, const Object::CollectionPtr& collection) {
  // Build atom array
  bsoncxx::builder::basic::array lhsArray;
  for (const auto& id : lhs) {
    lhsArray.append(id.bsoncxx());
  }
  bsoncxx::builder::basic::array rhsArray;
  for (const auto& id : rhs) {
    rhsArray.append(id.bsoncxx());
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

Reaction Reaction::create(const std::vector<ID>& lhs, const std::vector<ID>& rhs, const CollectionPtr& collection) {
  if (!collection) {
    throw Exceptions::MissingCollectionException();
  }

  return {createImpl(lhs, rhs, collection), collection};
}

ID Reaction::create(const std::vector<ID>& lhs, const std::vector<ID>& rhs) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  this->_id = std::make_unique<ID>(createImpl(lhs, rhs, _collection));
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
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
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
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
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

void Reaction::addReactant(const ID& id, const SIDE side) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  if (side == SIDE::BOTH || side == SIDE::LHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$push" << open_document
                               << "lhs" << id.bsoncxx()
                               << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    _collection->mongocxx().find_one_and_update(selection.view(), update.view());
  }
  if (side == SIDE::BOTH || side == SIDE::RHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$push" << open_document
                               << "rhs" << id.bsoncxx()
                               << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    _collection->mongocxx().find_one_and_update(selection.view(), update.view());
  }
}

void Reaction::removeReactant(const ID& id, const SIDE side) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  if (side == SIDE::BOTH || side == SIDE::LHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$pull" << open_document
                               << "lhs" << id.bsoncxx()
                               << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    _collection->mongocxx().find_one_and_update(selection.view(), update.view());
  }
  if (side == SIDE::BOTH || side == SIDE::RHS) {
    auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
    // clang-format off
    auto update = document{} << "$pull" << open_document
                               << "rhs" << id.bsoncxx()
                               << close_document
                             << "$currentDate" << open_document
                               << "_lastmodified" << true
                               << close_document
                             << finalize;
    // clang-format on
    _collection->mongocxx().find_one_and_update(selection.view(), update.view());
  }
}

void Reaction::setReactants(const std::vector<ID>& ids, const SIDE side) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  bsoncxx::builder::basic::array array;
  for (const auto& id : ids) {
    array.append(id.bsoncxx());
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
    _collection->mongocxx().find_one_and_update(selection.view(), update.view());
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
    _collection->mongocxx().find_one_and_update(selection.view(), update.view());
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
    lhs = Fields::Serialization<std::vector<ID>>::deserialize(view["lhs"]).value();
  }
  if (side == SIDE::BOTH || side == SIDE::RHS) {
    rhs = Fields::Serialization<std::vector<ID>>::deserialize(view["rhs"]).value();
  }
  return {lhs, rhs};
}

std::tuple<std::vector<Compound>, std::vector<Compound>> Reaction::getReactants(const SIDE side, const Manager& manager,
                                                                                const std::string& collection) const {
  auto reactantIds = getReactants(side);
  auto collectionPtr = manager.getCollection(collection);
  const auto transform = [&](std::vector<ID>& ids) -> std::vector<Compound> {
    std::vector<Compound> compounds;
    compounds.reserve(ids.size());
    for (auto& id : ids) {
      compounds.emplace_back(std::move(id), collectionPtr);
    }
    return compounds;
  };
  return std::make_tuple(transform(std::get<0>(reactantIds)), transform(std::get<1>(reactantIds)));
}

std::tuple<int, int> Reaction::hasReactants() const {
  auto reactants = this->getReactants(SIDE::BOTH);
  return {std::get<0>(reactants).size(), std::get<1>(reactants).size()};
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
    _collection->mongocxx().find_one_and_update(selection.view(), update.view());
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
    _collection->mongocxx().find_one_and_update(selection.view(), update.view());
  }
}

} /* namespace Database */
} /* namespace Scine */
