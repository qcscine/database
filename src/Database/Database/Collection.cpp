/**
 * @file Collection.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Includes*/
#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Objects/BoolProperty.h"
#include "Database/Objects/Calculation.h"
#include "Database/Objects/Compound.h"
#include "Database/Objects/DenseMatrixProperty.h"
#include "Database/Objects/ElementaryStep.h"
#include "Database/Objects/Flask.h"
#include "Database/Objects/NumberProperty.h"
#include "Database/Objects/Object.h"
#include "Database/Objects/Property.h"
#include "Database/Objects/Reaction.h"
#include "Database/Objects/SparseMatrixProperty.h"
#include "Database/Objects/StringProperty.h"
#include "Database/Objects/Structure.h"
#include "Database/Objects/VectorProperty.h"
/* External Includes */
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <cassert>
#include <memory>
#include <mongocxx/collection.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

namespace Scine {
namespace Database {

Collection::Collection(mongocxx::collection base)
  : _collection(std::make_unique<mongocxx::collection>(std::move(base))) {
}

Collection::~Collection() = default;

Collection::Collection(const Collection& other) : enable_shared_from_this(other) {
  this->_collection = std::make_unique<mongocxx::collection>(*other._collection);
}

Collection::Collection(Collection&& other) noexcept = default;

Collection& Collection::operator=(const Collection& rhs) {
  this->_collection = std::make_unique<mongocxx::collection>(*rhs._collection);
  return *this;
}

Collection& Collection::operator=(Collection&& rhs) noexcept = default;

template<class ObjectClass>
ObjectClass Collection::get(ID id) {
  // sanity check
  static_assert(std::is_base_of<Object, ObjectClass>::value, "Requested class is not a SCINE database object.");
  auto selection = document{} << "_id" << id.bsoncxx() << finalize;
  mongocxx::options::find options;
  options.no_cursor_timeout(true);
  auto projection = document{} << "_id" << 1 << "_objecttype" << 1 << finalize;
  options.projection(projection.view());
  auto optional = this->_collection->find_one(selection.view(), options);
  if (!optional) {
    throw Exceptions::IDNotFoundException();
  }
  return ObjectClass{std::move(id), shared_from_this()};
}

template<class ObjectClass>
ObjectClass Collection::getAndUpdateOne(const std::string& filter, const std::string& update, const std::string& sort) {
  // sanity check
  static_assert(std::is_base_of<Object, ObjectClass>::value, "Requested class is not a SCINE database object.");
  auto selectionDoc = bsoncxx::from_json(filter);
  auto updateDoc = bsoncxx::from_json(update);
  auto projection = document{} << "_id" << 1 << "_objecttype" << 1 << finalize;
  auto options = mongocxx::options::find_one_and_update();
  options.projection(projection.view());
  const std::string sortDocInput = (sort.empty()) ? "{}" : sort;
  auto sortDoc = bsoncxx::from_json(sortDocInput);
  if (!sort.empty()) {
    options.sort(sortDoc.view());
  }
  auto optional = this->_collection->find_one_and_update(selectionDoc.view(), updateDoc.view(), options);
  if (!optional) {
    return ObjectClass();
  }
  auto doc = optional.value().view();
  auto objecttype = doc["_objecttype"].get_utf8().value;
  using View = decltype(objecttype);
  auto id = doc["_id"].get_oid().value;
  if (!(std::is_base_of<Object, ObjectClass>::value && (objecttype == View(ObjectClass::objecttype)))) {
    throw Exceptions::ObjectTypeMismatchException();
  }

  return ObjectClass{id, shared_from_this()};
}

template<class ObjectClass>
ObjectClass Collection::getOne(const std::string& filter, const std::string& sort) {
  // sanity check
  static_assert(std::is_base_of<Object, ObjectClass>::value, "Requested class is not a SCINE database object.");
  auto selectionDoc = bsoncxx::from_json(filter);
  mongocxx::options::find options;
  options.no_cursor_timeout(true);
  auto projection = document{} << "_id" << 1 << "_objecttype" << 1 << finalize;
  options.projection(projection.view());
  const std::string sortDocInput = (sort.empty()) ? "{}" : sort;
  auto sortDoc = bsoncxx::from_json(sortDocInput);
  if (!sort.empty()) {
    options.sort(sortDoc.view());
  }
  auto optional = this->_collection->find_one(selectionDoc.view(), options);
  if (!optional) {
    return ObjectClass();
  }
  auto doc = optional.value().view();
  auto objecttype = doc["_objecttype"].get_utf8().value;
  using View = decltype(objecttype);
  auto id = doc["_id"].get_oid().value;
  if (!(std::is_base_of<Object, ObjectClass>::value && (objecttype == View(ObjectClass::objecttype)))) {
    throw Exceptions::ObjectTypeMismatchException();
  }

  return ObjectClass{id, shared_from_this()};
}

bool Collection::has(const ID& id) {
  auto selection = document{} << "_id" << id.bsoncxx() << finalize;
  mongocxx::options::find options;
  options.no_cursor_timeout(true);
  auto projection = document{} << "_id" << 1 << "_objecttype" << 1 << finalize;
  options.projection(projection.view());
  auto optional = this->_collection->find_one(selection.view(), options);
  return static_cast<bool>(optional);
}

template<class ObjectClass>
bool Collection::has(const ID& id) {
  auto selection = document{} << "_id" << id.bsoncxx() << finalize;
  mongocxx::options::find options;
  options.no_cursor_timeout(true);
  auto projection = document{} << "_id" << 1 << "_objecttype" << 1 << finalize;
  options.projection(projection.view());
  auto optional = this->_collection->find_one(selection.view(), options);
  if (!optional) {
    return false;
  }

  // if id has been found ...
  auto doc = optional.value().view();
  auto objecttype = doc["_objecttype"].get_utf8().value;
  using View = decltype(objecttype);
  return std::is_base_of<Object, ObjectClass>::value && (objecttype == View(ObjectClass::objecttype));
}

boost::optional<ID> Collection::find(const std::string& query) {
  mongocxx::options::find options;
  options.no_cursor_timeout(true);
  auto projection = document{} << "_id" << 1 << "_objecttype" << 1 << finalize;
  options.projection(projection.view());
  auto optional = this->_collection->find_one(bsoncxx::from_json(query), options);
  if (!optional) {
    return boost::none;
  }

  auto document = optional.value().view();
  auto id = document["_id"].get_oid().value;
  return ID{id};
}

template<class ObjectClass>
std::vector<ObjectClass> Collection::query(const std::string& selection) {
  static_assert(std::is_base_of<Object, ObjectClass>::value, "Requested class is not a SCINE database object.");
  mongocxx::options::find options;
  options.no_cursor_timeout(true);
  auto projection = document{} << "_id" << 1 << "_objecttype" << 1 << finalize;
  options.projection(projection.view());
  mongocxx::cursor cursor = this->_collection->find(bsoncxx::from_json(selection), options);
  return toVector<ObjectClass>(cursor);
}

template<class ObjectClass>
std::vector<ObjectClass> Collection::randomSelect(std::int32_t nSamples) {
  static_assert(std::is_base_of<Object, ObjectClass>::value, "Requested class is not a SCINE database object.");
  mongocxx::pipeline pipeline = mongocxx::pipeline();
  mongocxx::cursor cursor = this->_collection->aggregate(pipeline.sample(nSamples));
  return toVector<ObjectClass>(cursor);
}

template<class ObjectClass>
std::vector<ObjectClass> Collection::toVector(mongocxx::v_noabi::cursor& cursor) {
  std::vector<ObjectClass> vec;
  for (const auto& doc : cursor) {
    auto objecttype = doc["_objecttype"].get_utf8().value;
    using View = decltype(objecttype);
    if (objecttype == View(ObjectClass::objecttype)) {
      vec.emplace_back(ID(doc["_id"].get_oid().value), shared_from_this());
    }
  }
  return vec;
}

unsigned int Collection::count(const std::string& selection) {
  return this->_collection->count_documents(bsoncxx::from_json(selection));
}

template<class ObjectClass>
Collection::CollectionLooper<ObjectClass> Collection::iteratorQuery(const std::string& selection) {
  return CollectionLooper<ObjectClass>(*this, bsoncxx::from_json(selection));
}

template<class Obj>
struct Collection::CollectionLooper<Obj>::Impl {
  Impl(Collection& coll, bsoncxx::document::view_or_value query, mongocxx::options::find opts)
    : _coll(coll.shared_from_this()),
      _cur(std::make_shared<mongocxx::cursor>(coll.mongocxx().find(query, opts))),
      _ptr(_cur->begin()),
      _end(_cur->end()) {
  }
  std::shared_ptr<Collection> _coll;
  std::shared_ptr<mongocxx::cursor> _cur;
  mongocxx::cursor::iterator _ptr;
  mongocxx::cursor::iterator _end;
};

template<class Obj>
Collection::CollectionLooper<Obj>::CollectionLooper(Collection& coll, bsoncxx::document::view_or_value query) {
  mongocxx::options::find options;
  options.no_cursor_timeout(true);
  auto projection = document{} << "_id" << 1 << "_objecttype" << 1 << finalize;
  options.projection(projection.view());
  _pImpl = std::make_unique<Impl>(coll, query, options);
}

template<class Obj>
Collection::CollectionLooper<Obj>::CollectionLooper::~CollectionLooper() = default;

template<class Obj>
Collection::CollectionLooper<Obj>::CollectionLooper(const CollectionLooper<Obj>& other)
  : _pImpl(std::make_unique<Impl>(*other._pImpl)) {
}

template<class Obj>
Collection::CollectionLooper<Obj>::CollectionLooper(CollectionLooper<Obj>&&) noexcept = default;

template<class Obj>
Collection::CollectionLooper<Obj>& Collection::CollectionLooper<Obj>::operator=(const CollectionLooper<Obj>& rhs) {
  *_pImpl = *rhs._pImpl;
  return *this;
}

template<class Obj>
Collection::CollectionLooper<Obj>& Collection::CollectionLooper<Obj>::operator=(CollectionLooper<Obj>&&) noexcept = default;

template<class Obj>
Obj Collection::CollectionLooper<Obj>::operator*() {
  auto& mongoIter = *_pImpl->_ptr;
  auto id = mongoIter["_id"].get_value().get_oid().value;
  return Obj(ID(id), _pImpl->_coll);
}

template<class Obj>
Collection::CollectionLooper<Obj>& Collection::CollectionLooper<Obj>::operator++() {
  (_pImpl->_ptr)++;
  return *this;
}

template<class Obj>
Collection::CollectionLooper<Obj> Collection::CollectionLooper<Obj>::operator++(int /*dummy*/) {
  CollectionLooper i = *this;
  (_pImpl->_ptr)++;
  return i;
}

template<class Obj>
bool Collection::CollectionLooper<Obj>::done() const {
  return (_pImpl->_ptr) == (_pImpl->_end);
}

mongocxx::collection& Collection::mongocxx() {
  return *_collection;
}

template class Collection::CollectionLooper<Calculation>;
template class Collection::CollectionLooper<Compound>;
template class Collection::CollectionLooper<ElementaryStep>;
template class Collection::CollectionLooper<Reaction>;
template class Collection::CollectionLooper<Structure>;
template class Collection::CollectionLooper<Flask>;
template class Collection::CollectionLooper<Property>;
template class Collection::CollectionLooper<BoolProperty>;
template class Collection::CollectionLooper<NumberProperty>;
template class Collection::CollectionLooper<SparseMatrixProperty>;
template class Collection::CollectionLooper<StringProperty>;
template class Collection::CollectionLooper<VectorProperty>;
template class Collection::CollectionLooper<DenseMatrixProperty>;

template Calculation Collection::get<Calculation>(ID id);
template Compound Collection::get<Compound>(ID id);
template ElementaryStep Collection::get<ElementaryStep>(ID id);
template Reaction Collection::get<Reaction>(ID id);
template Structure Collection::get<Structure>(ID id);
template Flask Collection::get<Flask>(ID id);
template Property Collection::get<Property>(ID id);
template BoolProperty Collection::get<BoolProperty>(ID id);
template NumberProperty Collection::get<NumberProperty>(ID id);
template SparseMatrixProperty Collection::get<SparseMatrixProperty>(ID id);
template StringProperty Collection::get<StringProperty>(ID id);
template VectorProperty Collection::get<VectorProperty>(ID id);
template DenseMatrixProperty Collection::get<DenseMatrixProperty>(ID id);

template bool Collection::has<Calculation>(const ID& id);
template bool Collection::has<Compound>(const ID& id);
template bool Collection::has<ElementaryStep>(const ID& id);
template bool Collection::has<Reaction>(const ID& id);
template bool Collection::has<Structure>(const ID& id);
template bool Collection::has<Flask>(const ID& id);
template bool Collection::has<Property>(const ID& id);
template bool Collection::has<BoolProperty>(const ID& id);
template bool Collection::has<NumberProperty>(const ID& id);
template bool Collection::has<SparseMatrixProperty>(const ID& id);
template bool Collection::has<StringProperty>(const ID& id);
template bool Collection::has<VectorProperty>(const ID& id);
template bool Collection::has<DenseMatrixProperty>(const ID& id);

template Collection::CollectionLooper<Calculation> Collection::iteratorQuery<Calculation>(const std::string& selection);
template Collection::CollectionLooper<Compound> Collection::iteratorQuery<Compound>(const std::string& selection);
template Collection::CollectionLooper<ElementaryStep> Collection::iteratorQuery<ElementaryStep>(const std::string& selection);
template Collection::CollectionLooper<Reaction> Collection::iteratorQuery<Reaction>(const std::string& selection);
template Collection::CollectionLooper<Structure> Collection::iteratorQuery<Structure>(const std::string& selection);
template Collection::CollectionLooper<Flask> Collection::iteratorQuery<Flask>(const std::string& selection);
template Collection::CollectionLooper<Property> Collection::iteratorQuery<Property>(const std::string& selection);
template Collection::CollectionLooper<BoolProperty> Collection::iteratorQuery<BoolProperty>(const std::string& selection);
template Collection::CollectionLooper<NumberProperty> Collection::iteratorQuery<NumberProperty>(const std::string& selection);
template Collection::CollectionLooper<SparseMatrixProperty>
Collection::iteratorQuery<SparseMatrixProperty>(const std::string& selection);
template Collection::CollectionLooper<StringProperty> Collection::iteratorQuery<StringProperty>(const std::string& selection);
template Collection::CollectionLooper<VectorProperty> Collection::iteratorQuery<VectorProperty>(const std::string& selection);
template Collection::CollectionLooper<DenseMatrixProperty>
Collection::iteratorQuery<DenseMatrixProperty>(const std::string& selection);

template std::vector<Calculation> Collection::query<Calculation>(const std::string& selection);
template std::vector<Compound> Collection::query<Compound>(const std::string& selection);
template std::vector<ElementaryStep> Collection::query<ElementaryStep>(const std::string& selection);
template std::vector<Reaction> Collection::query<Reaction>(const std::string& selection);
template std::vector<Structure> Collection::query<Structure>(const std::string& selection);
template std::vector<Flask> Collection::query<Flask>(const std::string& selection);
template std::vector<Property> Collection::query<Property>(const std::string& selection);
template std::vector<BoolProperty> Collection::query<BoolProperty>(const std::string& selection);
template std::vector<NumberProperty> Collection::query<NumberProperty>(const std::string& selection);
template std::vector<SparseMatrixProperty> Collection::query<SparseMatrixProperty>(const std::string& selection);
template std::vector<StringProperty> Collection::query<StringProperty>(const std::string& selection);
template std::vector<VectorProperty> Collection::query<VectorProperty>(const std::string& selection);
template std::vector<DenseMatrixProperty> Collection::query<DenseMatrixProperty>(const std::string& selection);

template std::vector<Calculation> Collection::randomSelect<Calculation>(std::int32_t nSamples);
template std::vector<Compound> Collection::randomSelect<Compound>(std::int32_t nSamples);
template std::vector<ElementaryStep> Collection::randomSelect<ElementaryStep>(std::int32_t nSamples);
template std::vector<Reaction> Collection::randomSelect<Reaction>(std::int32_t nSamples);
template std::vector<Structure> Collection::randomSelect<Structure>(std::int32_t nSamples);
template std::vector<Flask> Collection::randomSelect<Flask>(std::int32_t nSamples);
template std::vector<Property> Collection::randomSelect<Property>(std::int32_t nSamples);
template std::vector<BoolProperty> Collection::randomSelect<BoolProperty>(std::int32_t nSamples);
template std::vector<NumberProperty> Collection::randomSelect<NumberProperty>(std::int32_t nSamples);
template std::vector<SparseMatrixProperty> Collection::randomSelect<SparseMatrixProperty>(std::int32_t nSamples);
template std::vector<StringProperty> Collection::randomSelect<StringProperty>(std::int32_t nSamples);
template std::vector<VectorProperty> Collection::randomSelect<VectorProperty>(std::int32_t nSamples);
template std::vector<DenseMatrixProperty> Collection::randomSelect<DenseMatrixProperty>(std::int32_t nSamples);

template Calculation Collection::getOne<Calculation>(const std::string& filter, const std::string& sort);
template Compound Collection::getOne<Compound>(const std::string& filter, const std::string& sort);
template ElementaryStep Collection::getOne<ElementaryStep>(const std::string& filter, const std::string& sort);
template Reaction Collection::getOne<Reaction>(const std::string& filter, const std::string& sort);
template Structure Collection::getOne<Structure>(const std::string& filter, const std::string& sort);
template Flask Collection::getOne<Flask>(const std::string& filter, const std::string& sort);
template Property Collection::getOne<Property>(const std::string& filter, const std::string& sort);
template BoolProperty Collection::getOne<BoolProperty>(const std::string& filter, const std::string& sort);
template NumberProperty Collection::getOne<NumberProperty>(const std::string& filter, const std::string& sort);
template SparseMatrixProperty Collection::getOne<SparseMatrixProperty>(const std::string& filter, const std::string& sort);
template StringProperty Collection::getOne<StringProperty>(const std::string& filter, const std::string& sort);
template VectorProperty Collection::getOne<VectorProperty>(const std::string& filter, const std::string& sort);
template DenseMatrixProperty Collection::getOne<DenseMatrixProperty>(const std::string& filter, const std::string& sort);

template Calculation Collection::getAndUpdateOne<Calculation>(const std::string& filter, const std::string& update,
                                                              const std::string& sort);
template Compound Collection::getAndUpdateOne<Compound>(const std::string& filter, const std::string& update,
                                                        const std::string& sort);
template ElementaryStep Collection::getAndUpdateOne<ElementaryStep>(const std::string& filter,
                                                                    const std::string& update, const std::string& sort);
template Reaction Collection::getAndUpdateOne<Reaction>(const std::string& filter, const std::string& update,
                                                        const std::string& sort);
template Structure Collection::getAndUpdateOne<Structure>(const std::string& filter, const std::string& update,
                                                          const std::string& sort);
template Flask Collection::getAndUpdateOne<Flask>(const std::string& filter, const std::string& update, const std::string& sort);
template Property Collection::getAndUpdateOne<Property>(const std::string& filter, const std::string& update,
                                                        const std::string& sort);
template BoolProperty Collection::getAndUpdateOne<BoolProperty>(const std::string& filter, const std::string& update,
                                                                const std::string& sort);
template NumberProperty Collection::getAndUpdateOne<NumberProperty>(const std::string& filter,
                                                                    const std::string& update, const std::string& sort);
template SparseMatrixProperty Collection::getAndUpdateOne<SparseMatrixProperty>(const std::string& filter,
                                                                                const std::string& update,
                                                                                const std::string& sort);
template StringProperty Collection::getAndUpdateOne<StringProperty>(const std::string& filter,
                                                                    const std::string& update, const std::string& sort);
template VectorProperty Collection::getAndUpdateOne<VectorProperty>(const std::string& filter,
                                                                    const std::string& update, const std::string& sort);
template DenseMatrixProperty Collection::getAndUpdateOne<DenseMatrixProperty>(const std::string& filter,
                                                                              const std::string& update,
                                                                              const std::string& sort);

} /* namespace Database */
} /* namespace Scine */
