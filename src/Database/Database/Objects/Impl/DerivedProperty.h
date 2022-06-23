/**
 * @file DerivedProperty.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef DATABASE_DERIVED_PROPERTY_H
#define DATABASE_DERIVED_PROPERTY_H

#include "Database/Collection.h"
#include "Database/Exceptions.h"
#include "Database/Id.h"
#include "Database/Objects/Model.h"
#include "Database/Objects/Property.h"
#include <Eigen/Core>
#include <Eigen/Sparse>
#include <boost/optional.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <memory>
#include <mongocxx/collection.hpp>
#include <type_traits>

namespace Scine {
namespace Database {
namespace Serialization {

template<typename T>
struct Serializer {
  template<typename Builder>
  static void serialize(Builder& /* b */, const T& /* data */) {
    throw std::runtime_error("Not implemented!");
  }

  static void project(mongocxx::options::find& /* options */) {
    throw std::runtime_error("Not implemented!");
  }

  static T deserialize(bsoncxx::document::view /* view */) {
    throw std::runtime_error("Not implemented!");
  }
};

//! NumberProperty: double
template<>
struct Serializer<double> {
  template<typename Builder>
  static void serialize(Builder& b, const double data) {
    b.append(bsoncxx::builder::basic::kvp("data", bsoncxx::types::b_double{data}));
  }

  static void project(mongocxx::options::find& options) {
    using namespace bsoncxx::builder::stream;
    options.projection(document{} << "data" << 1 << finalize);
  }

  static double deserialize(bsoncxx::document::view view) {
    if (view["data"].type() != bsoncxx::types::b_double::type_id) {
      throw Exceptions::MissingIdOrField();
    }
    return view["data"].get_double();
  }
};

//! BoolProperty: bool
template<>
struct Serializer<bool> {
  template<typename Builder>
  static void serialize(Builder& b, bool data) {
    b.append(bsoncxx::builder::basic::kvp("data", bsoncxx::types::b_bool{data}));
  }

  static void project(mongocxx::options::find& options) {
    using namespace bsoncxx::builder::stream;
    options.projection(document{} << "data" << 1 << finalize);
  }

  static bool deserialize(bsoncxx::document::view view) {
    if (view["data"].type() != bsoncxx::types::b_bool::type_id) {
      throw Exceptions::MissingIdOrField();
    }
    return view["data"].get_bool();
  }
};

//! StringProperty: std::string
template<>
struct Serializer<std::string> {
  template<typename Builder>
  static void serialize(Builder& b, const std::string& data) {
    b.append(bsoncxx::builder::basic::kvp("data", bsoncxx::types::b_utf8{data}));
  }

  static void project(mongocxx::options::find& options) {
    using namespace bsoncxx::builder::stream;
    options.projection(document{} << "data" << 1 << finalize);
  }

  static std::string deserialize(bsoncxx::document::view view) {
    if (view["data"].type() != bsoncxx::types::b_utf8::type_id) {
      throw Exceptions::MissingIdOrField();
    }
    return static_cast<std::string>(view["data"].get_utf8().value);
  }
};

/* VectorProperty: Eigen::VectorxXd */
template<>
struct Serializer<Eigen::VectorXd> {
  template<typename Builder>
  static void serialize(Builder& b, const Eigen::VectorXd& data) {
    using namespace bsoncxx::builder::basic;
    b.append(kvp("size", bsoncxx::types::b_int64{data.size()}));
    b.append(kvp("data", [&data](sub_array array) {
      const Eigen::Index len = data.size();
      for (Eigen::Index i = 0; i < len; ++i) {
        array.append(data.data()[i]);
      }
    }));
  }

  static void project(mongocxx::options::find& options) {
    using namespace bsoncxx::builder::stream;
    options.projection(document{} << "data" << 1 << "size" << 1 << finalize);
  }

  static Eigen::VectorXd deserialize(bsoncxx::document::view view) {
    const Eigen::Index size = view["size"].get_int64();
    Eigen::VectorXd ret(size);
    bsoncxx::array::view data = view["data"].get_array();
    Eigen::Index idx = 0;
    for (const auto it : data) {
      ret.data()[idx] = it.get_double();
      ++idx;
    }
    return ret;
  }
};

/* DenseMatrixProperty: Eigen::MatrixXd */
template<>
struct Serializer<Eigen::MatrixXd> {
  template<typename Builder>
  static void serialize(Builder& b, const Eigen::MatrixXd& data) {
    using namespace bsoncxx::builder::basic;
    b.append(kvp("cols", bsoncxx::types::b_int64{data.cols()}));
    b.append(kvp("rows", bsoncxx::types::b_int64{data.rows()}));
    b.append(kvp("data", [&data](sub_array array) {
      const Eigen::Index len = data.cols() * data.rows();
      for (Eigen::Index i = 0; i < len; ++i) {
        array.append(data.data()[i]);
      }
    }));
  }

  static void project(mongocxx::options::find& options) {
    using namespace bsoncxx::builder::stream;
    options.projection(document{} << "data" << 1 << "cols" << 1 << "rows" << 1 << finalize);
  }

  static Eigen::MatrixXd deserialize(bsoncxx::document::view view) {
    const Eigen::Index cols = view["cols"].get_int64();
    const Eigen::Index rows = view["rows"].get_int64();
    Eigen::MatrixXd ret(rows, cols);
    bsoncxx::array::view data = view["data"].get_array();
    Eigen::Index idx = 0;
    for (const auto it : data) {
      ret.data()[idx] = it.get_double();
      ++idx;
    }
    return ret;
  }
};

/* SparseMatrixProperty: Eigen::SparseMatrix<double> */
template<>
struct Serializer<Eigen::SparseMatrix<double>> {
  template<typename Builder>
  static void serialize(Builder& b, const Eigen::SparseMatrix<double>& data) {
    using namespace bsoncxx::builder::basic;
    b.append(kvp("cols", bsoncxx::types::b_int64{data.cols()}));
    b.append(kvp("rows", bsoncxx::types::b_int64{data.rows()}));
    b.append(kvp("size", bsoncxx::types::b_int64{data.nonZeros()}));
    b.append(kvp("data", [&data](sub_document document) {
      array row_idxs{};
      array col_idxs{};
      array values{};

      const Eigen::Index outer = data.outerSize();
      for (Eigen::Index i = 0; i < outer; ++i) {
        using Iterator = Eigen::SparseMatrix<double>::InnerIterator;
        for (Iterator it(data, i); it; ++it) {
          row_idxs.append(it.row());
          col_idxs.append(it.col());
          values.append(it.value());
        }
      }

      document.append(kvp("row_idxs", row_idxs));
      document.append(kvp("col_idxs", col_idxs));
      document.append(kvp("values", values));
    }));
  }

  static void project(mongocxx::options::find& options) {
    using namespace bsoncxx::builder::stream;
    options.projection(document{} << "data" << 1 << "size" << 1 << "cols" << 1 << "rows" << 1 << finalize);
  }

  static Eigen::SparseMatrix<double> deserialize(bsoncxx::document::view view) {
    const Eigen::Index cols = view["cols"].get_int64();
    const Eigen::Index rows = view["rows"].get_int64();
    const Eigen::Index size = view["size"].get_int64();

    // Read triplets
    std::vector<Eigen::Triplet<double>> triplets;
    triplets.reserve(size);
    bsoncxx::document::view data = view["data"].get_document();
    bsoncxx::array::view values = data["values"].get_array();
    bsoncxx::array::view col_idxs = data["col_idxs"].get_array();
    bsoncxx::array::view row_idxs = data["row_idxs"].get_array();
    auto values_it = values.begin();
    auto col_it = col_idxs.begin();
    auto row_it = row_idxs.begin();
    while (values_it != values.end() and col_it != col_idxs.end() and row_it != row_idxs.end()) {
      auto v = *values_it++;
      auto c = *col_it++;
      auto r = *row_it++;
      triplets.emplace_back(r.get_int64(), c.get_int64(), v.get_double());
    }

    Eigen::SparseMatrix<double> ret(rows, cols);
    ret.setFromTriplets(triplets.begin(), triplets.end());
    return ret;
  }
};

} /* namespace Serialization */

namespace DerivedProperty {

template<typename DerivedType, typename T>
DerivedType create(std::shared_ptr<Collection> collection, const Model& model, const std::string& name, const T& data,
                   boost::optional<ID> structureId, boost::optional<ID> calculationId) {
  static_assert(std::is_base_of<Property, DerivedType>::value, "Class is not a derived class of Property!");
  using namespace bsoncxx::builder::basic;
  if (!collection) {
    throw Exceptions::MissingCollectionException();
  }

  document builder{};
  const bsoncxx::types::b_date now{std::chrono::system_clock::now()};
  builder.append(kvp("_created", now));
  builder.append(kvp("_lastmodified", now));
  builder.append(kvp("analysis_disabled", false));
  builder.append(kvp("exploration_disabled", false));
  builder.append(kvp("_objecttype", DerivedType::objecttype));
  builder.append(kvp("_propertytype", DerivedType::propertytype));
  builder.append(kvp("model", model.toBson()));
  builder.append(kvp("property_name", name));
  Serialization::Serializer<T>::serialize(builder, data);
  builder.append(kvp("comment", ""));
  if (structureId) {
    builder.append(kvp("structure", structureId->bsoncxx()));
  }
  if (calculationId) {
    builder.append(kvp("calculation", calculationId->bsoncxx()));
  }

  auto doc = builder.extract();
  auto result = collection->mongocxx().insert_one(doc.view());
  ID id{result->inserted_id().get_oid().value};
  return DerivedType{std::move(id), collection};
}

template<typename DerivedProperty, typename T>
void updateData(DerivedProperty& derived, const T& data) {
  static_assert(std::is_base_of<Property, DerivedProperty>::value, "Class is not a derived class of Property!");
  auto collection = derived.collection();
  if (!collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }
  using namespace bsoncxx::builder::basic;
  document selection{};
  selection.append(kvp("_id", derived.id().bsoncxx()));
  document update{};
  update.append(kvp("$set", [&data](sub_document updates) { Serialization::Serializer<T>::serialize(updates, data); }));
  update.append(kvp("$currentDate", [](sub_document dates) { dates.append(kvp("_lastmodified", true)); }));
  collection->mongocxx().find_one_and_update(selection.view(), update.view());
}

template<typename T, typename DerivedProperty>
T getData(DerivedProperty& derived) {
  static_assert(std::is_base_of<Property, DerivedProperty>::value, "Class is not a derived class of Property!");

  auto collection = derived.collection();
  if (!collection) {
    throw Exceptions::MissingLinkedCollectionException();
  }

  using namespace bsoncxx::builder::basic;
  document selection{};
  selection.append(kvp("_id", derived.id().bsoncxx()));
  mongocxx::options::find options{};
  Serialization::Serializer<T>::project(options);
  auto maybeDocument = collection->mongocxx().find_one(selection.view(), options);
  if (!maybeDocument) {
    throw Exceptions::MissingIdOrField();
  }
  auto view = maybeDocument.value().view();
  return Serialization::Serializer<T>::deserialize(view);
}

} /* namespace DerivedProperty */
} /* namespace Database */
} /* namespace Scine */

#endif
