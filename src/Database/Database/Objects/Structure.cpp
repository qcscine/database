/**
 * @file Structure.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Includes */
#include "Database/Objects/Structure.h"
#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Manager.h"
#include "Database/Objects/Calculation.h"
#include "Database/Objects/Impl/Fields.h"
#include "Database/Objects/Model.h"
/* External Includes */
#include <Utils/Geometry/AtomCollection.h>
#include <Utils/Geometry/ElementInfo.h>
#include <bsoncxx/builder/concatenate.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <mongocxx/collection.hpp>

using bsoncxx::builder::concatenate;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::open_document;

namespace Scine {
namespace Database {
namespace {

ID createImpl(const Utils::AtomCollection& atoms, const int charge, const int multiplicity, const Model& model,
              const Structure::LABEL label, const Object::CollectionPtr& collection) {
  assert(collection);

  // Build atom array
  auto pos = atoms.getPositions();
  auto elements = atoms.getElements();
  bsoncxx::builder::basic::array atomArray;
  for (int i = 0; i < atoms.size(); i++) {
    // clang-format off
    auto doc = document{} << "element" << Utils::ElementInfo::symbol(elements[i])
                          << "x" << pos(i,0)
                          << "y" << pos(i,1)
                          << "z" << pos(i,2)
                          << finalize;
    // clang-format on
    atomArray.append(doc);
  }

  auto now = bsoncxx::types::b_date(std::chrono::system_clock::now());
  // clang-format off
  auto doc = document{} << "_created" << now
                        << "_lastmodified" << now
                        << "_objecttype" << Structure::objecttype
                        << "analysis_disabled" << false
                        << "exploration_disabled" << false
                        << "model" << model.toBson()
                        << "nAtoms"  << atoms.size()
                        << "atoms" << atomArray
                        << "charge" << charge
                        << "multiplicity" << multiplicity
                        << "label" << Layout::EnumMaps::label2str.at(label)
                        << "properties" << open_document << close_document
                        << "calculations" << open_document << close_document
                        << "comment" << ""
                        << "aggregate" << ""
                        << "duplicate_of" << ""
                        << "graphs" << open_document << close_document
                        << finalize;
  // clang-format on
  auto result = collection->mongocxx().insert_one(doc.view());
  return {result->inserted_id().get_oid().value};
}

} // namespace

constexpr const char* Structure::objecttype;

Structure Structure::create(const Utils::AtomCollection& atoms, const int charge, const int multiplicity,
                            const CollectionPtr& collection) {
  if (!collection) {
    throw Exceptions::MissingCollectionException();
  }

  return Structure{createImpl(atoms, charge, multiplicity, Model{"guess", "", "", "any"}, LABEL::NONE, collection), collection};
}

Structure Structure::create(const Utils::AtomCollection& atoms, const int charge, const int multiplicity,
                            const Model& model, const LABEL label, const CollectionPtr& collection) {
  if (!collection) {
    throw Exceptions::MissingCollectionException();
  }

  return Structure{createImpl(atoms, charge, multiplicity, model, label, collection), collection};
}

ID Structure::create(const Utils::AtomCollection& atoms, const int charge, const int multiplicity) {
  Structure::LABEL label = Structure::LABEL::NONE;
  Model model("guess", "", "", "any");
  return this->create(atoms, charge, multiplicity, model, label);
}

ID Structure::create(const Utils::AtomCollection& atoms, const int charge, const int multiplicity, const Model& model,
                     const LABEL label) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  this->_id = std::make_unique<ID>(createImpl(atoms, charge, multiplicity, model, label, _collection));
  return *(this->_id);
}

/*=========*
 *  Atoms
 *=========*/

Utils::AtomCollection Structure::getAtoms() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  // Select and find
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "nAtoms" << 1 << "atoms" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  // Check and prepare
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  // Read number of atoms
  auto nAtoms = view["nAtoms"].get_int32();
  // Read atoms in AtomCollection
  Utils::AtomCollection atoms;
  atoms.resize(nAtoms);
  bsoncxx::array::view atomsView = view["atoms"].get_array();
  for (int i = 0; i < nAtoms; i++) {
    const auto atom = atomsView[i];
    std::string symbol = atom["element"].get_utf8().value.to_string();
    const auto e = Utils::ElementInfo::elementTypeForSymbol(symbol);
    const double x = atom["x"].get_double();
    const double y = atom["y"].get_double();
    const double z = atom["z"].get_double();
    atoms.setElement(i, e);
    atoms.setPosition(i, Eigen::Vector3d(x, y, z));
  }
  return atoms;
}

void Structure::setAtoms(const Utils::AtomCollection& atoms) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  // Build atom array
  auto pos = atoms.getPositions();
  auto elements = atoms.getElements();
  bsoncxx::builder::basic::array atomArray;
  for (int i = 0; i < atoms.size(); i++) {
    // clang-format off
    auto doc = document{} << "element" << Utils::ElementInfo::symbol(elements[i])
                          << "x" << pos(i,0)
                          << "y" << pos(i,1)
                          << "z" << pos(i,2)
                          << finalize;
    // clang-format on
    atomArray.append(doc);
  }
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "nAtoms"  << atoms.size()
                             << "atoms" << atomArray
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

int Structure::hasAtoms() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  // Select and find
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "nAtoms" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  // Check and prepare
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  if (view["nAtoms"].type() != bsoncxx::types::b_int32::type_id)
    throw Exceptions::MissingIdOrField();
  return view["nAtoms"].get_int32();
}

void Structure::clearAtoms() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "nAtoms"  << 0
                             << "atoms" << open_array << close_array
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

/*=========*
 * Various
 *=========*/

Model Structure::getModel() const {
  return Fields::get<Model>(*this, "model");
}

void Structure::setModel(const Model& model) const {
  Fields::set(*this, "model", model);
}

Structure::LABEL Structure::getLabel() const {
  return Fields::get<Structure::LABEL>(*this, "label");
}

void Structure::setLabel(const LABEL label) const {
  Fields::set(*this, "label", label);
}

int Structure::getCharge() const {
  return Fields::get<int>(*this, "charge");
}

void Structure::setCharge(const int charge) const {
  Fields::set(*this, "charge", charge);
}

int Structure::getMultiplicity() const {
  return Fields::get<int>(*this, "multiplicity");
}

void Structure::setMultiplicity(const int multiplicity) const {
  Fields::set(*this, "multiplicity", multiplicity);
}

/*============*
 *  Compound
 *============*/

ID Structure::getAggregate(bool recursive) const {
  if (!recursive) {
    return Fields::get<ID>(*this, "aggregate");
  }
  // check if field has entry
  bool nonNull = Fields::nonNull(*this, "aggregate");
  if (nonNull || !hasOriginal()) {
    return Fields::get<ID>(*this, "aggregate");
  }
  auto original = Structure(getOriginal(), _collection);
  return original.getAggregate();
}

bool Structure::hasAggregate(bool recursive) const {
  // check if field has entry
  bool nonNull = Fields::nonNull(*this, "aggregate");
  if (!recursive || !hasOriginal()) {
    return nonNull;
  }
  // we now know that we have an original that we can get
  auto original = Structure(getOriginal(), _collection);
  return original.hasAggregate();
}

void Structure::setAggregate(const ID& id) const {
  Fields::set(*this, "aggregate", id);
}

void Structure::clearAggregate() const {
  Fields::set(*this, "aggregate", std::string{});
}

ID Structure::getCompound() const {
  return this->getAggregate();
}

bool Structure::hasCompound() const {
  return this->hasAggregate();
}

void Structure::setCompound(const ID& id) const {
  this->setAggregate(id);
}

void Structure::clearCompound() const {
  this->clearAggregate();
}

/*============*
 *  Property
 *============*/

bool Structure::hasProperty(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  const auto properties = this->getAllProperties();
  return properties.count(key) > 0;
}

bool Structure::hasProperty(const ID& id) const {
  // Get all execute search on client side.
  auto properties = this->getAllProperties();
  for (const auto& props : properties) {
    if (std::find(props.second.begin(), props.second.end(), id) != props.second.end())
      return true;
  }
  return false;
}

ID Structure::getProperty(const std::string& key) const {
  auto vector = this->getProperties(key);
  if (vector.size() != 1)
    throw Exceptions::FieldException();
  return vector[0];
}

void Structure::setProperty(const std::string& key, const ID& id) const {
  this->setProperties(key, {id});
}

void Structure::addProperty(const std::string& key, const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$push" << open_document
                             << "properties."+key << id.bsoncxx()
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Structure::removeProperty(const std::string& key, const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$pull" << open_document
                             << "properties."+key << id.bsoncxx()
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Structure::setProperties(const std::string& key, const std::vector<ID>& ids) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  bsoncxx::builder::basic::array array;
  for (const auto& id : ids) {
    array.append(id.bsoncxx());
  }
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "properties."+key << array
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

std::vector<ID> Structure::getProperties(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "properties" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto doc = view["properties"].get_document().view();
  auto findIter = doc.find(key);
  if (findIter == doc.end()) {
    return {};
  }
  bsoncxx::array::view id_array = findIter->get_array();
  std::vector<ID> ret;
  for (bsoncxx::array::element ele : id_array) {
    ret.emplace_back(ele.get_oid().value);
  }
  return ret;
}

std::vector<ID> Structure::queryProperties(const std::string& key, const Model& model,
                                           std::shared_ptr<Collection> collection) const {
  // Return empty list if the key is not even present
  const auto allProperties = this->getAllProperties();
  const auto it = allProperties.find(key);
  if (it == allProperties.end())
    return {};
  const auto& ids = it->second;

  // Setup selection for query in properties
  bsoncxx::builder::basic::array array;
  for (const auto& id : ids) {
    array.append(id.bsoncxx());
  }

  auto selection = document{} << "_id" << open_document << "$in" << array << close_document << finalize;
  auto cursor = collection->mongocxx().find(selection.view());
  std::vector<ID> ret;
  for (const auto& doc : cursor) {
    Model docModel(doc["model"].get_document().view());
    if (docModel == model) {
      ret.emplace_back(doc["_id"].get_oid().value);
    }
  }
  return ret;
}

int Structure::hasProperties(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto properties = this->getAllProperties();
  if (properties.count(key) == 0)
    return 0;
  return properties[key].size();
}

void Structure::clearProperties(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$unset" << open_document
                             << "properties."+key << ""
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

std::map<std::string, std::vector<ID>> Structure::getAllProperties() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "properties" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto doc = view["properties"].get_document().view();
  std::map<std::string, std::vector<ID>> ret;
  for (bsoncxx::document::element ele : doc) {
    std::vector<ID> tmp;
    bsoncxx::array::view array = ele.get_array();
    for (bsoncxx::array::element entry : array) {
      tmp.emplace_back(entry.get_oid().value);
    }
    ret[ele.key().to_string()] = tmp;
  }
  return ret;
}

void Structure::setAllProperties(const std::map<std::string, std::vector<ID>>& properties) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  bsoncxx::builder::basic::document doc;
  for (const auto& data : properties) {
    bsoncxx::builder::basic::array array;
    for (const auto& id : data.second) {
      array.append(id.bsoncxx());
    }
    doc.append(bsoncxx::builder::basic::kvp(data.first, array));
  }
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "properties" << doc
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Structure::clearAllProperties() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "properties" << open_document << close_document
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

/*================*
 *  Calculations  *
 *================*/

bool Structure::hasCalculation(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  const auto calculations = this->getAllCalculations();
  return calculations.count(key) > 0;
}

bool Structure::hasCalculation(const ID& id) const {
  // Get all execute search on client side.
  auto calculations = this->getAllCalculations();
  for (const auto& calcs : calculations) {
    if (std::find(calcs.second.begin(), calcs.second.end(), id) != calcs.second.end())
      return true;
  }
  return false;
}

ID Structure::getCalculation(const std::string& key) const {
  auto vector = this->getCalculations(key);
  if (vector.size() != 1)
    throw Exceptions::FieldException();
  return vector[0];
}

void Structure::setCalculation(const std::string& key, const ID& id) const {
  this->setCalculations(key, {id});
}

void Structure::addCalculation(const std::string& key, const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$push" << open_document
                           << "calculations."+key << id.bsoncxx()
                           << close_document
                           << "$currentDate" << open_document
                           << "_lastmodified" << true
                           << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Structure::addCalculations(const std::string& key, const std::vector<ID>& ids) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  bsoncxx::builder::basic::array array;
  for (const auto& id : ids) {
    array.append(id.bsoncxx());
  }
  // clang-format off
  auto update = document{} << "$push" << open_document
                           << "calculations."+key << open_document << "$each" << array
                           << close_document << close_document
                           << "$currentDate" << open_document
                           << "_lastmodified" << true
                           << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Structure::removeCalculation(const std::string& key, const ID& id) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$pull" << open_document
                           << "calculations."+key << id.bsoncxx()
                           << close_document
                           << "$currentDate" << open_document
                           << "_lastmodified" << true
                           << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Structure::setCalculations(const std::string& key, const std::vector<ID>& ids) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  bsoncxx::builder::basic::array array;
  for (const auto& id : ids) {
    array.append(id.bsoncxx());
  }
  // clang-format off
  auto update = document{} << "$set" << open_document
                           << "calculations."+key << array
                           << close_document
                           << "$currentDate" << open_document
                           << "_lastmodified" << true
                           << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

std::vector<ID> Structure::getCalculations(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "calculations" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto doc = view["calculations"].get_document().view();
  auto findIter = doc.find(key);
  if (findIter == doc.end()) {
    return {};
  }
  bsoncxx::array::view id_array = findIter->get_array();
  std::vector<ID> ret;
  for (bsoncxx::array::element ele : id_array) {
    ret.emplace_back(ele.get_oid().value);
  }
  return ret;
}

std::vector<ID> Structure::queryCalculations(const std::string& key, const Model& model,
                                             std::shared_ptr<Collection> collection) const {
  // Return empty list if the key is not even present
  const auto allCalculations = this->getAllCalculations();
  const auto it = allCalculations.find(key);
  if (it == allCalculations.end())
    return {};
  const auto& ids = it->second;

  // Setup selection for query in calculations
  bsoncxx::builder::basic::array array;
  for (const auto& id : ids) {
    array.append(id.bsoncxx());
  }

  auto selection = document{} << "_id" << open_document << "$in" << array << close_document << finalize;
  auto cursor = collection->mongocxx().find(selection.view());
  std::vector<ID> ret;
  for (const auto& doc : cursor) {
    Model docModel(doc["model"].get_document().view());
    if (docModel == model) {
      ret.emplace_back(doc["_id"].get_oid().value);
    }
  }
  return ret;
}

int Structure::hasCalculations(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto calculations = this->getAllCalculations();
  if (calculations.count(key) == 0)
    return 0;
  return calculations[key].size();
}

void Structure::clearCalculations(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$unset" << open_document
                           << "calculations."+key << ""
                           << close_document
                           << "$currentDate" << open_document
                           << "_lastmodified" << true
                           << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

std::map<std::string, std::vector<ID>> Structure::getAllCalculations() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "calculations" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto doc = view["calculations"].get_document().view();
  std::map<std::string, std::vector<ID>> ret;
  for (bsoncxx::document::element ele : doc) {
    std::vector<ID> tmp;
    bsoncxx::array::view array = ele.get_array();
    for (bsoncxx::array::element entry : array) {
      tmp.emplace_back(entry.get_oid().value);
    }
    ret[ele.key().to_string()] = tmp;
  }
  return ret;
}

void Structure::setAllCalculations(const std::map<std::string, std::vector<ID>>& calculations) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  bsoncxx::builder::basic::document doc;
  for (const auto& data : calculations) {
    bsoncxx::builder::basic::array array;
    for (const auto& id : data.second) {
      array.append(id.bsoncxx());
    }
    doc.append(bsoncxx::builder::basic::kvp(data.first, array));
  }
  // clang-format off
  auto update = document{} << "$set" << open_document
                           << "calculations" << doc
                           << close_document
                           << "$currentDate" << open_document
                           << "_lastmodified" << true
                           << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Structure::clearAllCalculations() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                           << "calculations" << open_document << close_document
                           << close_document
                           << "$currentDate" << open_document
                           << "_lastmodified" << true
                           << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

/*===================*
 *  Molecular Graph
 *===================*/

std::string Structure::getGraph(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "graphs." + key << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto graphs = view["graphs"].get_document().view();
  auto findIter = graphs.find(key);
  if (findIter == graphs.end()) {
    throw Exceptions::MissingIdOrField();
  }
  return findIter->get_utf8().value.to_string();
}

void Structure::setGraph(const std::string& key, const std::string& graph) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "graphs."+key << graph
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Structure::removeGraph(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$unset" << open_document
                             << "graphs."+key << ""
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

bool Structure::hasGraph(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  const auto graphs = this->getGraphs();
  return graphs.count(key) > 0;
}

int Structure::hasGraphs() const {
  return this->getGraphs().size();
}

void Structure::clearGraphs() const {
  this->setGraphs({});
}

std::map<std::string, std::string> Structure::getGraphs() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "graphs" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto doc = view["graphs"].get_document().view();
  std::map<std::string, std::string> ret;
  for (bsoncxx::document::element ele : doc) {
    ret[ele.key().to_string()] = ele.get_utf8().value.to_string();
  }
  return ret;
}

void Structure::setGraphs(const std::map<std::string, std::string>& graphs) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  bsoncxx::builder::basic::document doc;
  for (const auto& data : graphs) {
    doc.append(bsoncxx::builder::basic::kvp(data.first, data.second));
  }
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "graphs" << doc
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

/*===========*
 *  Comment
 *===========*/

std::string Structure::getComment() const {
  return Fields::get<std::string>(*this, "comment");
}

void Structure::setComment(const std::string& comment) const {
  Fields::set(*this, "comment", comment);
}

bool Structure::hasComment() const {
  return Fields::nonNull(*this, "comment");
}

void Structure::clearComment() const {
  this->setComment("");
}

/*==============*
 *  Duplicates
 *==============*/

bool Structure::hasOriginal() const {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }
  return Fields::nonNull(*this, "duplicate_of");
}

ID Structure::getOriginal() const {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }
  if (!this->hasOriginal()) {
    throw Exceptions::MissingIdOrField();
  }
  return Fields::get<ID>(*this, "duplicate_of");
}

void Structure::setOriginal(const ID& id) const {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }
  Fields::set<ID>(*this, "duplicate_of", id);
}

void Structure::clearOriginal() const {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }
  Fields::set(*this, "duplicate_of", std::string{});
}

ID Structure::isDuplicateOf() const {
  return getOriginal();
}

void Structure::setAsDuplicateOf(const ID& id) const {
  setOriginal(id);
}

void Structure::clearDuplicateID() const {
  clearOriginal();
}

} /* namespace Database */
} /* namespace Scine */
