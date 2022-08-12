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
#include <Database/Objects/ReactionEnums.h>
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
  // Build arrays
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
                        << "idx_maps" << open_document << close_document
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

std::tuple<double, double> ElementaryStep::getBarrierFromSpline() const {
  if (not this->hasSpline())
    return {0.0, 0.0};
  const auto& spline = this->getSpline();
  const Eigen::MatrixXd& data = spline.data;
  const Eigen::VectorXd& knots = spline.knots;
  const unsigned int tsDataIndex = (knots.array() <= spline.tsPosition).count() - 1;

  const double lhs_energy = data(0, 0);
  const double ts_energy = data(tsDataIndex, 0);
  const double rhs_energy = data(data.rows() - 1, 0);

  return {ts_energy - lhs_energy, ts_energy - rhs_energy};
}

void ElementaryStep::setType(const ElementaryStepType& type) const {
  Fields::set(*this, "type", type);
}

ElementaryStepType ElementaryStep::getType() const {
  return Fields::get<ElementaryStepType>(*this, "type");
}

/*==============*
 *  Index Maps
 *==============*/

void ElementaryStep::addIdxMaps(const std::vector<int>& lhsRhsMap, const boost::optional<std::vector<int>>& lhsTsMap) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  if (lhsTsMap) {
    // clang-format off
      auto update = document{} << "$set" << open_document
                                << "idx_maps.lhs_rhs" << Fields::Serialization<std::vector<int>>::serialize(lhsRhsMap)
                                << "idx_maps.lhs_ts" << Fields::Serialization<std::vector<int>>::serialize(lhsTsMap.get())
                                << close_document
                              << "$currentDate" << open_document
                                << "_lastmodified" << true
                                << close_document
                              << finalize;
    // clang-format on
    _collection->mongocxx().find_one_and_update(selection.view(), update.view());
  }
  else {
    // clang-format off
      auto update = document{} << "$set" << open_document
                                << "idx_maps.lhs_rhs" << Fields::Serialization<std::vector<int>>::serialize(lhsRhsMap)
                                << close_document
                              << "$currentDate" << open_document
                                << "_lastmodified" << true
                                << close_document
                              << finalize;
    // clang-format on
    _collection->mongocxx().find_one_and_update(selection.view(), update.view());
  }
}

void ElementaryStep::removeIdxMaps() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$unset" << open_document
                              << "idx_maps.lhs_rhs" << open_array << close_array
                              << "idx_maps.lhs_ts" << open_array << close_array
                              << close_document
                            << "$currentDate" << open_document
                              << "_lastmodified" << true
                              << close_document
                            << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

bool ElementaryStep::hasIdxMap(const IdxMapType& mapType) const {
  if (mapType == IdxMapType::LHS_TS || mapType == IdxMapType::TS_LHS) {
    return this->hasIdxMapByKey("lhs_ts");
  }
  else if (mapType == IdxMapType::LHS_RHS || mapType == IdxMapType::RHS_LHS) {
    return this->hasIdxMapByKey("lhs_rhs");
  }
  else if (mapType == IdxMapType::TS_RHS || mapType == IdxMapType::RHS_TS) {
    return (this->hasIdxMapByKey("lhs_ts") && this->hasIdxMapByKey("lhs_rhs"));
  }
  else {
    // Should not be reachable
    throw std::runtime_error("No method for specified map type.");
  }
}

std::vector<int> ElementaryStep::getIdxMap(const IdxMapType& mapType) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  // Directly retrievable maps
  else if (mapType == IdxMapType::LHS_RHS) {
    return this->getIdxMapByKey("lhs_rhs");
  }
  else if (mapType == IdxMapType::LHS_TS) {
    return this->getIdxMapByKey("lhs_ts");
  }
  // Simply reversed maps
  else if (mapType == IdxMapType::RHS_LHS) {
    std::vector<int> lhsRhs = getIdxMapByKey("lhs_rhs");
    return this->reverseIdxMap(lhsRhs);
  }
  else if (mapType == IdxMapType::TS_LHS) {
    std::vector<int> lhsTs = getIdxMapByKey("lhs_ts");
    return this->reverseIdxMap(lhsTs);
  }
  // Chained maps
  else if (mapType == IdxMapType::TS_RHS) {
    std::vector<int> lhsTs = getIdxMapByKey("lhs_ts");
    std::vector<int> lhsRhs = getIdxMapByKey("lhs_rhs");
    return this->chainIdxMaps(this->reverseIdxMap(lhsTs), lhsRhs);
  }
  else if (mapType == IdxMapType::RHS_TS) {
    std::vector<int> lhsTs = getIdxMapByKey("lhs_ts");
    std::vector<int> lhsRhs = getIdxMapByKey("lhs_rhs");
    return this->chainIdxMaps(this->reverseIdxMap(lhsRhs), lhsTs);
  }
  else {
    // Should not be reachable
    throw std::runtime_error("No getter for specified map type.");
  }
}

bool ElementaryStep::hasIdxMapByKey(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  // clang-format off
  auto selection = document{} << "$and" << open_array
                              << open_document << "_id" << this->id().bsoncxx() << close_document
                              << open_document << "idx_maps."+key << open_document
                                << "$exists" << "true"
                                << close_document << close_document
                              << close_array
                              << finalize;
  // clang-format on
  auto optional = _collection->mongocxx().find_one(selection.view());
  return static_cast<bool>(optional);
}

std::vector<int> ElementaryStep::getIdxMapByKey(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "idx_maps" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  // Check and prepare
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  // Load results section
  auto mapIt = view["idx_maps"].get_document().view();
  auto findIter = mapIt.find(key);
  if (findIter == mapIt.end()) {
    throw Exceptions::MissingIdOrField();
  }
  bsoncxx::array::view wantedMapArray = findIter->get_array();
  std::vector<int> wantedMap;
  for (bsoncxx::array::element ele : wantedMapArray) {
    wantedMap.emplace_back(ele.get_int32());
  }
  return wantedMap;
}

std::vector<int> ElementaryStep::reverseIdxMap(const std::vector<int>& unswapped) const {
  int vectorSize = unswapped.size();
  std::vector<int> swapped(vectorSize, -1);
  for (int i = 0; i < vectorSize; ++i) {
    if (unswapped.at(i) > vectorSize) {
      throw InvalidIdxMapException();
    }
    swapped[unswapped.at(i)] = i;
  }
  // Make sure that all elements were set explicitly
  if (std::find(swapped.begin(), swapped.end(), -1) != swapped.end()) {
    throw InvalidIdxMapException();
  }
  return swapped;
}

std::vector<int> ElementaryStep::chainIdxMaps(const ::std::vector<int>& idxMap1, const ::std::vector<int>& idxMap2) const {
  if (idxMap1.size() != idxMap2.size()) {
    throw InvalidIdxMapException();
  }
  int mapSize = idxMap1.size();
  std::vector<int> chainedMap;
  for (int idx1 : idxMap1) {
    if (idx1 > mapSize) {
      throw InvalidIdxMapException();
    }
    if (idxMap2.at(idx1) > mapSize) {
      throw InvalidIdxMapException();
    }
    chainedMap.push_back(idxMap2.at(idx1));
  }
  return chainedMap;
}

/*==============*
 *     Path
 *==============*/

bool ElementaryStep::hasStructureInPath(const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  const auto structures = this->getPath();
  return std::find(structures.begin(), structures.end(), id) != structures.end();
}

int ElementaryStep::hasPath() const {
  return Fields::get<std::vector<ID>>(*this, "path").size();
}

std::vector<ID> ElementaryStep::getPath() const {
  return Fields::get<std::vector<ID>>(*this, "path");
}

std::vector<Structure> ElementaryStep::getPath(const Manager& manager, const std::string& collection) const {
  auto ids = getPath();
  std::vector<Structure> structures;
  auto structureCollection = manager.getCollection(collection);
  structures.reserve(ids.size());
  for (auto& id : ids) {
    structures.emplace_back(std::move(id), structureCollection);
  }
  return structures;
}

void ElementaryStep::setPath(const std::vector<ID>& ids) const {
  Fields::set(*this, "path", ids);
}

void ElementaryStep::clearPath() const {
  setPath({});
}

} /* namespace Database */
} /* namespace Scine */
