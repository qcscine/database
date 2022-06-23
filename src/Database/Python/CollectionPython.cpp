/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include "ObjectsVariant.h"

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

using namespace Scine::Database;

/// @cond
struct Impl {
  Impl(Collection& coll, const bsoncxx::document::view_or_value& query, const mongocxx::options::find& opts)
    : _cur(std::make_shared<mongocxx::cursor>(coll.mongocxx().find(query, opts))), _ptr(_cur->begin()), _end(_cur->end()) {
  }
  std::shared_ptr<mongocxx::cursor> _cur;
  mongocxx::cursor::iterator _ptr;
  mongocxx::cursor::iterator _end;
};

template<class T>
class CustomIterator {
  std::shared_ptr<Impl> _impl;

 public:
  CustomIterator(Collection& coll, bsoncxx::document::view_or_value query) {
    mongocxx::options::find opts;
    opts.no_cursor_timeout(true);
    _impl = std::make_shared<Impl>(coll, query, opts);
  }
  T operator*() {
    return T(ID((*(_impl->_ptr))["_id"].get_value().get_oid().value));
  }
  CustomIterator& operator++() {
    ++(_impl->_ptr);
    return *this;
  }
  bool done() const {
    return _impl->_ptr == _impl->_end;
  }
};

class CustomSentinel {};

template<class T>
bool operator==(const CustomIterator<T>& it, const CustomSentinel& /* sentinel */) {
  return it.done();
}
/// @endcond

template<class T>
pybind11::iterator iterateAll(Collection& coll) {
  auto query = document{} << finalize;
  return pybind11::make_iterator(CustomIterator<T>(coll, query.view()), CustomSentinel());
}

template<class T>
pybind11::iterator iterateQuery(Collection& coll, const std::string& query) {
  return pybind11::make_iterator(CustomIterator<T>(coll, bsoncxx::from_json(query)), CustomSentinel());
}

template<class T>
boost::optional<T> getOneWrapper(Collection& coll, std::string query, std::string sort = "") {
  auto ret = coll.getOne<T>(query, sort);
  if (ret.hasId()) {
    return ret;
  }

  return boost::none;
}

bool has_overload(Collection& coll, ID& id) {
  return coll.has(id);
}

using CollectionClass = pybind11::class_<Collection, std::shared_ptr<Collection>>;

void init_collection(CollectionClass& collection) {
  collection.def("has", &has_overload, pybind11::arg("id"));
  collection.def("has_calculation", &Collection::has<Calculation>, pybind11::arg("id"));
  collection.def("has_property", &Collection::has<Property>, pybind11::arg("id"));
  collection.def("has_structure", &Collection::has<Structure>, pybind11::arg("id"));
  collection.def("has_reaction", &Collection::has<Reaction>, pybind11::arg("id"));
  collection.def("has_elementary_step", &Collection::has<ElementaryStep>, pybind11::arg("id"));
  collection.def("has_compound", &Collection::has<Compound>, pybind11::arg("id"));

  collection.def(
      "find",
      [](Collection& coll, const std::string& query) -> boost::optional<DerivedObjectVariant> {
        if (auto idOptional = coll.find(query)) {
          return fetchLinked(coll, idOptional.value());
        }
        return boost::none;
      },
      R"delim(
      Finds a single arbitrary object in this collection by a query and returns
      it (linked to the current collection), if found. Returns ``None`` otherwise.
    )delim");

  collection.def("get_and_update_one_calculation", &Collection::getAndUpdateOne<Calculation>, pybind11::arg("filter"),
                 pybind11::arg("update") = "", pybind11::arg("sort") = "");
  collection.def("get_and_update_one_compound", &Collection::getAndUpdateOne<Compound>, pybind11::arg("filter"),
                 pybind11::arg("update") = "", pybind11::arg("sort") = "");
  collection.def("get_and_update_one_property", &Collection::getAndUpdateOne<Property>, pybind11::arg("filter"),
                 pybind11::arg("update") = "", pybind11::arg("sort") = "");
  collection.def("get_and_update_one_reaction", &Collection::getAndUpdateOne<Reaction>, pybind11::arg("filter"),
                 pybind11::arg("update") = "", pybind11::arg("sort") = "");
  collection.def("get_and_update_one_elementary_step", &Collection::getAndUpdateOne<ElementaryStep>,
                 pybind11::arg("filter"), pybind11::arg("update") = "", pybind11::arg("sort") = "");
  collection.def("get_and_update_one_structure", &Collection::getAndUpdateOne<Structure>, pybind11::arg("filter"),
                 pybind11::arg("update") = "", pybind11::arg("sort") = "");

  collection.def("get_one_calculation", &getOneWrapper<Calculation>, pybind11::arg("filter"), pybind11::arg("sort") = "");
  collection.def("get_one_compound", &getOneWrapper<Compound>, pybind11::arg("filter"), pybind11::arg("sort") = "");
  collection.def("get_one_property", &getOneWrapper<Property>, pybind11::arg("filter"), pybind11::arg("sort") = "");
  collection.def("get_one_reaction", &getOneWrapper<Reaction>, pybind11::arg("filter"), pybind11::arg("sort") = "");
  collection.def("get_one_elementary_step", &getOneWrapper<ElementaryStep>, pybind11::arg("filter"),
                 pybind11::arg("sort") = "");
  collection.def("get_one_structure", &getOneWrapper<Structure>, pybind11::arg("filter"), pybind11::arg("sort") = "");

  collection.def("get_calculation", &Collection::get<Calculation>, pybind11::arg("calculation"));
  collection.def("get_compound", &Collection::get<Compound>, pybind11::arg("compound"));
  collection.def("get_property", &Collection::get<Property>, pybind11::arg("property"));
  collection.def("get_number_property", &Collection::get<NumberProperty>, pybind11::arg("property"));
  collection.def("get_vector_property", &Collection::get<VectorProperty>, pybind11::arg("property"));
  collection.def("get_dense_matrix_property", &Collection::get<DenseMatrixProperty>, pybind11::arg("property"));
  collection.def("get_sparse_matrix_property", &Collection::get<SparseMatrixProperty>, pybind11::arg("property"));
  collection.def("get_reaction", &Collection::get<Reaction>, pybind11::arg("reaction"));
  collection.def("get_elementary_step", &Collection::get<ElementaryStep>, pybind11::arg("reaction_path"));
  collection.def("get_structure", &Collection::get<Structure>, pybind11::arg("structure"));

  collection.def("query_calculations", &Collection::query<Calculation>, pybind11::arg("selection"));
  collection.def("query_compounds", &Collection::query<Compound>, pybind11::arg("selection"));
  collection.def("query_properties", &Collection::query<Property>, pybind11::arg("selection"));
  collection.def("query_reactions", &Collection::query<Reaction>, pybind11::arg("selection"));
  collection.def("query_elementary_steps", &Collection::query<ElementaryStep>, pybind11::arg("selection"));
  collection.def("query_structures", &Collection::query<Structure>, pybind11::arg("selection"));

  collection.def("random_select_calculations", &Collection::randomSelect<Calculation>, pybind11::arg("n_samples"));
  collection.def("random_select_compounds", &Collection::randomSelect<Compound>, pybind11::arg("n_samples"));
  collection.def("random_select_properties", &Collection::randomSelect<Property>, pybind11::arg("n_samples"));
  collection.def("random_select_reactions", &Collection::randomSelect<Reaction>, pybind11::arg("n_samples"));
  collection.def("random_select_elementary_steps", &Collection::randomSelect<ElementaryStep>, pybind11::arg("n_samples"));
  collection.def("random_select_structures", &Collection::randomSelect<Structure>, pybind11::arg("n_samples"));

  collection.def("count", &Collection::count, pybind11::arg("selection"));

  collection.def("iterate_calculations", &iterateQuery<Calculation>, pybind11::arg("selection"));
  collection.def("iterate_compounds", &iterateQuery<Compound>, pybind11::arg("selection"));
  collection.def("iterate_properties", &iterateQuery<Property>, pybind11::arg("selection"));
  collection.def("iterate_reactions", &iterateQuery<Reaction>, pybind11::arg("selection"));
  collection.def("iterate_elementary_steps", &iterateQuery<ElementaryStep>, pybind11::arg("selection"));
  collection.def("iterate_structures", &iterateQuery<Structure>, pybind11::arg("selection"));

  collection.def("iterate_all_calculations", &iterateAll<Calculation>);
  collection.def("iterate_all_compounds", &iterateAll<Compound>);
  collection.def("iterate_all_properties", &iterateAll<Property>);
  collection.def("iterate_all_reactions", &iterateAll<Reaction>);
  collection.def("iterate_all_elementary_steps", &iterateAll<ElementaryStep>);
  collection.def("iterate_all_structures", &iterateAll<Structure>);
}
