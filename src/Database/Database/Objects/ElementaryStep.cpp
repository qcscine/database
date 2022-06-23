/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Includes*/
#include "Database/Objects/ElementaryStep.h"
#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Manager.h"
#include <Database/Objects/Impl/DerivedProperty.h>
#include <Database/Objects/Impl/Fields.h>
#include <Database/Objects/Reaction.h>
#include <Database/Objects/ReactionSide.h>
#include <Database/Objects/Structure.h>
/* External Includes */
#include <Utils/Geometry/ElementInfo.h>
#include <Utils/Math/BSplines/ReactionProfileInterpolation.h>
#include <Utils/Typenames.h>
#include <bsoncxx/builder/basic/document.hpp>
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
                        << "_objecttype" << ElementaryStep::objecttype
                        << "lhs" << lhsArray
                        << "rhs" << rhsArray
                        << "transition_state" << ""
                        << "type" << "regular"
                        << "reaction" << ""
                        << "path" << open_array << close_array
                        << "spline" << ""
                        << finalize;
  // clang-format on
  auto result = collection->mongocxx().insert_one(doc.view());
  return {result->inserted_id().get_oid().value};
}

} // namespace

constexpr const char* ElementaryStep::objecttype;

ElementaryStep ElementaryStep::create(const std::vector<ID>& lhs, const std::vector<ID>& rhs,
                                      const Object::CollectionPtr& collection) {
  if (!collection) {
    throw Exceptions::MissingCollectionException();
  }

  return {createImpl(lhs, rhs, collection), collection};
}

ID ElementaryStep::create(const std::vector<ID>& lhs, const std::vector<ID>& rhs) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  this->_id = std::make_unique<ID>(createImpl(lhs, rhs, _collection));
  return *(this->_id);
}

/*============*
 *  Reaction
 *============*/

ID ElementaryStep::getReaction() const {
  return Fields::get<ID>(*this, "reaction");
}

Reaction ElementaryStep::getReaction(const Manager& manager, const std::string& collection) const {
  return manager.getCollection(collection)->get<Reaction>(getReaction());
}

bool ElementaryStep::hasReaction() const {
  return Fields::nonNull(*this, "reaction");
}

void ElementaryStep::setReaction(const ID& id) const {
  Fields::set(*this, "reaction", id);
}

void ElementaryStep::clearReaction() const {
  Fields::set(*this, "reaction", std::string{});
}

/*====================*
 *  Transition State
 *====================*/

ID ElementaryStep::getTransitionState() const {
  return Fields::get<ID>(*this, "transition_state");
}

Structure ElementaryStep::getTransitionState(const Manager& manager, const std::string& collection) const {
  return manager.getCollection(collection)->get<Structure>(getTransitionState());
}

bool ElementaryStep::hasTransitionState() const {
  return Fields::nonNull(*this, "transition_state");
}

void ElementaryStep::setTransitionState(const ID& id) const {
  Fields::set(*this, "transition_state", id);
}

void ElementaryStep::clearTransitionState() const {
  Fields::set(*this, "transition_state", std::string{});
}

/*==============*
 *  Structures
 *==============*/

SIDE ElementaryStep::hasReactant(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto reactants = this->getReactants(SIDE::BOTH);
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

void ElementaryStep::addReactant(const ID& id, const SIDE side) const {
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

void ElementaryStep::removeReactant(const ID& id, const SIDE side) const {
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

void ElementaryStep::setReactants(const std::vector<ID>& ids, const SIDE side) const {
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

std::tuple<std::vector<ID>, std::vector<ID>> ElementaryStep::getReactants(const SIDE side) const {
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
    for (bsoncxx::array::element ele : array) {
      lhs.emplace_back(ele.get_oid().value);
    }
  }
  if (side == SIDE::BOTH || side == SIDE::RHS) {
    auto array = view["rhs"].get_array().value;
    for (bsoncxx::array::element ele : array) {
      rhs.emplace_back(ele.get_oid().value);
    }
  }
  return {lhs, rhs};
}

std::tuple<int, int> ElementaryStep::hasReactants() const {
  auto reactants = this->getReactants(SIDE::BOTH);
  return {std::get<0>(reactants).size(), std::get<1>(reactants).size()};
}

void ElementaryStep::clearReactants(const SIDE side) const {
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

bool ElementaryStep::hasSpline() const {
  return Fields::nonNull(*this, "spline");
}

Utils::BSplines::TrajectorySpline ElementaryStep::getSpline() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();

  // Select and find
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "spline" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  // Check and prepare
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();

  // Get TS position
  auto tsPosition = view["spline"]["ts_position"].get_double();

  // Parse spline data arrays
  Utils::ElementTypeCollection elements;
  bsoncxx::document::view knotView = view["spline"]["knots"].get_document();
  auto knots = Serialization::Serializer<Eigen::VectorXd>::deserialize(knotView);
  bsoncxx::document::view dataView = view["spline"]["data"].get_document();
  auto data = Serialization::Serializer<Eigen::MatrixXd>::deserialize(dataView);
  bsoncxx::array::view elementView = view["spline"]["elements"].get_array();
  for (const auto it : elementView) {
    std::string symbol = it.get_utf8().value.to_string();
    elements.push_back(Utils::ElementInfo::elementTypeForSymbol(symbol));
  }
  return Utils::BSplines::TrajectorySpline(elements, knots, data, tsPosition);
}

void ElementaryStep::setSpline(const Utils::BSplines::TrajectorySpline& spline) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();

  // Generate element array
  bsoncxx::builder::basic::array elements;
  for (unsigned int i = 0; i < spline.elements.size(); i++) {
    elements.append(Utils::ElementInfo::symbol(spline.elements[i]));
  }
  bsoncxx::builder::basic::document knots{};
  Serialization::Serializer<Eigen::VectorXd>::serialize(knots, spline.knots);
  bsoncxx::builder::basic::document data{};
  Serialization::Serializer<Eigen::MatrixXd>::serialize(data, spline.data);

  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "spline" << open_document
                               << "ts_position" << spline.tsPosition
                               << "elements" << elements
                               << "knots" << knots
                               << "data" << data
                             << close_document
                           << close_document
                           << "$currentDate" << open_document
                           << "_lastmodified" << true
                           << close_document
                           << finalize;
  // clang-format on
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void ElementaryStep::clearSpline() const {
  Fields::set(*this, "spline", std::string{});
}

void ElementaryStep::setType(const ElementaryStepType& type) const {
  Fields::set(*this, "type", type);
}

ElementaryStepType ElementaryStep::getType() const {
  return Fields::get<ElementaryStepType>(*this, "type");
}

} /* namespace Database */
} /* namespace Scine */
