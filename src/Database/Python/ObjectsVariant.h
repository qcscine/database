/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef INCLUDE_DATABASE_PYTHON_OBJECTS_VARIANT_H
#define INCLUDE_DATABASE_PYTHON_OBJECTS_VARIANT_H

#include "Utils/Pybind.h"
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Objects/BoolProperty.h>
#include <Database/Objects/Calculation.h>
#include <Database/Objects/Compound.h>
#include <Database/Objects/DenseMatrixProperty.h>
#include <Database/Objects/ElementaryStep.h>
#include <Database/Objects/Flask.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/NumberProperty.h>
#include <Database/Objects/Object.h>
#include <Database/Objects/Property.h>
#include <Database/Objects/Reaction.h>
#include <Database/Objects/ReactionEnums.h>
#include <Database/Objects/SparseMatrixProperty.h>
#include <Database/Objects/StringProperty.h>
#include <Database/Objects/Structure.h>
#include <Database/Objects/ValueCollection.h>
#include <Database/Objects/VectorProperty.h>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <mongocxx/collection.hpp>

namespace Scine {
namespace Database {

using DerivedObjectVariant =
    boost::variant<BoolProperty, Calculation, Compound, DenseMatrixProperty, ElementaryStep, NumberProperty, Reaction,
                   SparseMatrixProperty, StringProperty, Structure, VectorProperty, Flask>;

using PropertiesVariant =
    boost::variant<BoolProperty, DenseMatrixProperty, NumberProperty, SparseMatrixProperty, StringProperty, VectorProperty>;

template<typename ReturnVariant, typename StringLike>
ReturnVariant fetchProperty(const StringLike& propertytype, Collection& coll, const ID& id) {
  if (propertytype == StringLike(BoolProperty::propertytype)) {
    return BoolProperty{id, coll.shared_from_this()};
  }
  if (propertytype == StringLike(NumberProperty::propertytype)) {
    return NumberProperty{id, coll.shared_from_this()};
  }
  if (propertytype == StringLike(DenseMatrixProperty::propertytype)) {
    return DenseMatrixProperty{id, coll.shared_from_this()};
  }
  if (propertytype == StringLike(SparseMatrixProperty::propertytype)) {
    return SparseMatrixProperty{id, coll.shared_from_this()};
  }
  if (propertytype == StringLike(StringProperty::propertytype)) {
    return StringProperty{id, coll.shared_from_this()};
  }
  if (propertytype == StringLike(VectorProperty::propertytype)) {
    return VectorProperty{id, coll.shared_from_this()};
  }

  throw std::runtime_error("Unhandled property type!");
}

inline DerivedObjectVariant fetchLinked(Collection& coll, const ID& id) {
  using bsoncxx::builder::stream::document;
  using bsoncxx::builder::stream::finalize;

  auto selection = document{} << "_id" << id.bsoncxx() << finalize;
  auto optional = coll.mongocxx().find_one(selection.view());
  if (!optional) {
    throw Exceptions::IDNotFoundException();
  }
  auto doc = optional.value().view();
  const auto objecttype = doc["_objecttype"].get_utf8().value;
  using View = decltype(objecttype);

  if (objecttype == View(Property::objecttype)) {
    const auto propertytype = doc["_propertytype"].get_utf8().value;
    return fetchProperty<DerivedObjectVariant>(propertytype, coll, id);
  }

  if (objecttype == View(Calculation::objecttype)) {
    return Calculation{id, coll.shared_from_this()};
  }
  if (objecttype == View(Compound::objecttype)) {
    return Compound{id, coll.shared_from_this()};
  }
  if (objecttype == View(ElementaryStep::objecttype)) {
    return ElementaryStep{id, coll.shared_from_this()};
  }
  if (objecttype == View(Reaction::objecttype)) {
    return Reaction{id, coll.shared_from_this()};
  }
  if (objecttype == View(Structure::objecttype)) {
    return Structure{id, coll.shared_from_this()};
  }

  throw Exceptions::ObjectTypeMismatchException();
}

} // namespace Database
} // namespace Scine

#endif
