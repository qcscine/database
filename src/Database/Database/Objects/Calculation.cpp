/**
 * @file Calculation.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Class Header */
#include "Database/Objects/Calculation.h"
/* Internal Include */
#include "Database/Exceptions.h"
#include "Database/Manager.h"
#include "Database/Objects/Impl/Fields.h"
#include "Database/Objects/Model.h"
#include "Database/Objects/Structure.h"
#include "Database/Objects/ValueCollection.h"
/* External Includes */
#include <Utils/Geometry/ElementInfo.h>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
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

ID createImpl(const Model& model, const Calculation::Job& job, const std::vector<ID>& structures,
              const Object::CollectionPtr& collection) {
  // Build structures document
  bsoncxx::builder::basic::array str;
  for (auto const& ele : structures) {
    str.append(ele.bsoncxx());
  }
  auto now = bsoncxx::types::b_date(std::chrono::system_clock::now());
  // clang-format off
  auto doc = document{} << "_created" << now
                        << "_lastmodified" << now
                        << "_objecttype" << Calculation::objecttype
                        << "analysis_disabled" << false
                        << "exploration_disabled" << false
                        << "job" << open_document
                          << "order" << job.order
                          << "memory" << job.memory
                          << "cores" << job.cores
                          << "disk" << job.disk
                          << close_document
                        << "model" << model.toBson()
                        << "structures"  << str
                        << "auxiliaries" << open_document << close_document
                        << "settings" << open_document << close_document
                        << "status" << "construction"
                        << "priority" << 10
                        << "results" << open_document
                          << "properties" << open_array << close_array
                          << "structures" << open_array << close_array
                          << "elementary_steps" << open_array << close_array
                          << close_document
                        << "raw_output" << ""
                        << "comment" << ""
                        << "executor" << ""
                        << "runtime" << ""
                        << finalize;
  // clang-format on
  auto result = collection->mongocxx().insert_one(doc.view());
  return {result->inserted_id().get_oid().value};
}

} // namespace

constexpr const char* Calculation::objecttype;

Calculation Calculation::create(const Model& model, const Calculation::Job& job, const std::vector<ID>& structures,
                                const CollectionPtr& collection) {
  if (!collection) {
    throw Exceptions::MissingCollectionException();
  }

  return {createImpl(model, job, structures, collection), collection};
}

ID Calculation::create(const Model& model, const Calculation::Job& job, const std::vector<ID>& structures) {
  if (!_collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  this->_id = std::make_unique<ID>(createImpl(model, job, structures, _collection));
  return *(this->_id);
}

/*============*
 *  Priority
 *============*/

unsigned Calculation::getPriority() const {
  return static_cast<unsigned>(Fields::get<int>(*this, "priority"));
}

void Calculation::setPriority(unsigned priority) const {
  if (priority > 10 || priority < 1) {
    throw std::invalid_argument("Priority has to be between 1 and 10");
  }

  Fields::set(*this, "priority", static_cast<int>(priority));
}

/*==========*
 *  Status
 *==========*/

Calculation::STATUS Calculation::getStatus() const {
  return Fields::get<Calculation::STATUS>(*this, "status");
}

void Calculation::setStatus(Calculation::STATUS status) const {
  Fields::set(*this, "status", status);
}

/*=======*
 *  Job
 *=======*/

Calculation::Job Calculation::getJob() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  // Select and find
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "job" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  // Check and prepare
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  auto jobDoc = view["job"].get_document().view();
  // Populate
  Calculation::Job job("dummy");
  job.order = jobDoc["order"].get_utf8().value.to_string();
  job.memory = jobDoc["memory"].get_double();
  job.cores = jobDoc["cores"].get_int32();
  job.disk = jobDoc["disk"].get_double();
  return job;
}

void Calculation::setJob(const Calculation::Job& job) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "job" << open_document
                               << "order" << job.order
                               << "memory" << job.memory
                               << "cores" << job.cores
                               << "disk" << job.disk
                               << close_document
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

/*=========*
 *  Model
 *=========*/

Model Calculation::getModel() const {
  return Fields::get<Model>(*this, "model");
}

void Calculation::setModel(const Model& model) const {
  Fields::set(*this, "model", model);
}

/*==============*
 *  Strutures
 *==============*/

void Calculation::addStructure(const ID& id) const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // Build update document
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

void Calculation::removeStructure(const ID& id) const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // Build update document
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

bool Calculation::hasStructure(const ID& id) const {
  if (!this->exists())
    throw Exceptions::MissingLinkedCollectionException();
  //  auto view = this->getRawContent().view();
  auto structure_ids = this->getStructures(); // Serialization::GenericValue::deserializeIDArray(view["structures"].get_value().get_array());
  return std::find(structure_ids.begin(), structure_ids.end(), id) != structure_ids.end();
}

std::vector<ID> Calculation::getStructures() const {
  return Fields::get<std::vector<ID>>(*this, "structures");
}

std::vector<Structure> Calculation::getStructures(const Manager& manager, const std::string& collection) const {
  auto ids = getStructures();
  auto structureCollection = manager.getCollection(collection);
  std::vector<Structure> structures;
  structures.reserve(ids.size());
  for (ID& id : ids) {
    structures.emplace_back(std::move(id), structureCollection);
  }
  return structures;
}

void Calculation::clearStructures() const {
  setStructures({});
}

void Calculation::setStructures(const std::vector<ID>& structures) const {
  Fields::set(*this, "structures", structures);
}

/*============*
 *  Settings
 *============*/

void Calculation::setSetting(const std::string& key, const Utils::UniversalSettings::GenericValue& value) const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;

  bsoncxx::builder::basic::document builder;
  Serialization::GenericValue::serialize(builder, key, value);

  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "settings." + key << builder.view()[key].get_value()
                             << close_document
                           << "$currentDate"
                             << open_document << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

Utils::UniversalSettings::GenericValue Calculation::getSetting(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  /* selection:
   *   { _id : <ID> }
   */
  // clang-format off
  auto selection = document{} << "_id" << this->id().bsoncxx()
                              << finalize;
  // clang-format on
  mongocxx::options::find options{};
  options.projection(document{} << "settings." + key << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  return Serialization::GenericValue::deserialize(view["settings"][key].get_value());
}

bool Calculation::hasSetting(const std::string& key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  const auto keys = this->getSettings().getKeys();
  return std::find(keys.begin(), keys.end(), key) != keys.end();
}

void Calculation::removeSetting(const std::string& key) const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // Build update document
  // clang-format off
  auto update = document{} << "$unset" << open_document
                             << "settings."+key << ""
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Calculation::clearSettings() const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // Build empty settings document
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "settings" << open_document << close_document
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

Utils::UniversalSettings::ValueCollection Calculation::getSettings() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();

  // Select and find
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "settings" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  // Check and prepare
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();

  // Load settings
  return Serialization::ValueCollection::deserialize(view["settings"].get_document());
}

void Calculation::setSettings(const Utils::UniversalSettings::ValueCollection& settings) const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "settings" << Serialization::ValueCollection::serialize(settings)
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

/*===========*
 *  Results
 *===========*/

void Calculation::setResults(Calculation::Results& results) const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // Build results
  auto result_properties = bsoncxx::builder::basic::array{};
  for (const auto& id : results.properties) {
    result_properties.append(id.bsoncxx());
  }
  auto result_structures = bsoncxx::builder::basic::array{};
  for (const auto& id : results.structures) {
    result_structures.append(id.bsoncxx());
  }
  auto result_elementarysteps = bsoncxx::builder::basic::array{};
  for (const auto& id : results.elementarySteps) {
    result_elementarysteps.append(id.bsoncxx());
  }
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "results" << open_document
                               << "properties" << result_properties
                               << "structures" << result_structures
                               << "elementary_steps" << result_elementarysteps
                               << close_document
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Calculation::clearResults() const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // Build empty results
  auto result_properties = bsoncxx::builder::basic::array{};
  auto result_structures = bsoncxx::builder::basic::array{};
  auto result_elementarysteps = bsoncxx::builder::basic::array{};
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "results" << open_document
                               << "properties" << result_properties
                               << "structures" << result_structures
                               << "elementary_steps" << result_elementarysteps
                               << close_document
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

Calculation::Results Calculation::getResults() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  // Select and find
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "results" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  // Check and prepare
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  // Load results section
  auto resultsDoc = view["results"].get_document().view();
  bsoncxx::array::view result_properties = resultsDoc["properties"].get_array();
  bsoncxx::array::view result_structures = resultsDoc["structures"].get_array();
  bsoncxx::array::view result_elementarysteps = resultsDoc["elementary_steps"].get_array();
  Results results;
  for (bsoncxx::array::element ele : result_properties) {
    results.properties.emplace_back(ele.get_oid().value);
  }
  for (bsoncxx::array::element ele : result_structures) {
    results.structures.emplace_back(ele.get_oid().value);
  }
  for (bsoncxx::array::element ele : result_elementarysteps) {
    results.elementarySteps.emplace_back(ele.get_oid().value);
  }
  return results;
}

/*===============*
 *  Auxiliaries
 *===============*/

void Calculation::setAuxiliary(std::string key, const ID& id) const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // Build update document
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "auxiliaries."+key << id.bsoncxx()
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

ID Calculation::getAuxiliary(std::string key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  /* selection:
   *   { _id : <ID> }
   */
  // clang-format off
  auto selection = document{} << "_id" << this->id().bsoncxx()
                              << finalize;
  // clang-format on
  mongocxx::options::find options{};
  options.projection(document{} << "auxiliaries." + key << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();
  return ID(view["auxiliaries"][key].get_oid().value);
}

bool Calculation::hasAuxiliary(std::string key) const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  const auto auxiliaries = this->getAuxiliaries();
  return auxiliaries.count(key) > 0;
}

void Calculation::removeAuxiliary(std::string key) const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // Build update document
  // clang-format off
  auto update = document{} << "$unset" << open_document
                             << "auxiliaries."+key << ""
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Calculation::setAuxiliaries(std::map<std::string, ID> auxiliaries) const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // Build auxiliaries document
  bsoncxx::builder::basic::document aux;
  for (auto const& ele : auxiliaries) {
    aux.append(bsoncxx::builder::basic::kvp(ele.first, ele.second.bsoncxx()));
  }
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "auxiliaries" << aux
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

void Calculation::clearAuxiliaries() const {
  // Selection
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  // Build empty auxiliaries document
  // clang-format off
  auto update = document{} << "$set" << open_document
                             << "auxiliaries" << open_document << close_document
                             << close_document
                           << "$currentDate" << open_document
                             << "_lastmodified" << true
                             << close_document
                           << finalize;
  // clang-format on
  _collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

std::map<std::string, ID> Calculation::getAuxiliaries() const {
  if (!_collection)
    throw Exceptions::MissingLinkedCollectionException();
  // Select and find
  auto selection = document{} << "_id" << this->id().bsoncxx() << finalize;
  mongocxx::options::find options{};
  options.projection(document{} << "auxiliaries" << 1 << finalize);
  auto optional = _collection->mongocxx().find_one(selection.view(), options);
  // Check and prepare
  if (!optional)
    throw Exceptions::MissingIdOrField();
  auto view = optional.value().view();

  // Load auxiliary IDs
  std::map<std::string, ID> auxiliaries;
  auto auxiliariesDoc = view["auxiliaries"].get_document().view();
  for (bsoncxx::document::element ele : auxiliariesDoc) {
    auxiliaries.emplace(ele.key().to_string(), ele.get_oid().value);
  }
  return auxiliaries;
}

/*==============*
 *  Raw Output
 *==============*/

std::string Calculation::getRawOutput() const {
  return Fields::get<std::string>(*this, "raw_output");
}

void Calculation::setRawOutput(const std::string& output) const {
  Fields::set(*this, "raw_output", output);
}

bool Calculation::hasRawOutput() const {
  return Fields::nonNull(*this, "raw_output");
}

void Calculation::clearRawOutput() const {
  this->setRawOutput("");
}

/*=============*
 *  Comment
 *=============*/

std::string Calculation::getComment() const {
  return Fields::get<std::string>(*this, "comment");
}

void Calculation::setComment(const std::string& comment) const {
  Fields::set(*this, "comment", comment);
}

bool Calculation::hasComment() const {
  return Fields::nonNull(*this, "comment");
}

void Calculation::clearComment() const {
  this->setComment("");
}

/*============*
 *  Executor
 *============*/

std::string Calculation::getExecutor() const {
  return Fields::get<std::string>(*this, "executor");
}

void Calculation::setExecutor(const std::string& executor) const {
  Fields::set(*this, "executor", executor);
}

bool Calculation::hasExecutor() const {
  return Fields::nonNull(*this, "executor");
}

void Calculation::clearExecutor() const {
  this->setExecutor("");
}

/*============*
 *  Runtime
 *============*/

double Calculation::getRuntime() const {
  return Fields::get<double>(*this, "runtime");
}

void Calculation::setRuntime(const double runtime) const {
  Fields::set(*this, "runtime", runtime);
}

bool Calculation::hasRuntime() const {
  return Fields::nonNull(*this, "runtime");
}

void Calculation::clearRuntime() const {
  Fields::set(*this, "runtime", std::string{});
}

} /* namespace Database */
} /* namespace Scine */
