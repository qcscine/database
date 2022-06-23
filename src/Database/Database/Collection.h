/**
 * @file Collection.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_COLLECTION_H_
#define DATABASE_COLLECTION_H_

/* External Includes */
#include <boost/optional.hpp>
#include <cassert>
#include <memory>
#include <vector>

namespace mongocxx {
inline namespace v_noabi {
class collection;
class cursor;
} // namespace v_noabi
} // namespace mongocxx
namespace bsoncxx {
inline namespace v_noabi {
template<class T, class U>
class view_or_value;
namespace document {
class view;
class value;
using view_or_value = bsoncxx::v_noabi::view_or_value<view, value>;
} // namespace document
} // namespace v_noabi
} // namespace bsoncxx

namespace Scine {
namespace Database {

class ID;

/**
 * @class Collection Collection.h
 * @brief A wrapper for the mongocxx::collection in order to add
 *        some functionalities.
 *
 * The purpose of this wrapper class is to enable easier access to
 * the classes defined as objects in the database namespace.
 * The access to specific types of these objects is done using
 * templates.
 */
class Collection : public std::enable_shared_from_this<Collection> {
 public:
  /**
   * @brief Construct a new Collection object.
   * @param base The mongocxx::collection base object.
   */
  Collection(mongocxx::v_noabi::collection base);
  /// @brief Destructor.
  ~Collection();
  /// @brief Copy constructor.
  Collection(const Collection& other);
  /// @brief Move constructor.
  Collection(Collection&& other) noexcept;
  /// @brief Assignment operator.
  Collection& operator=(const Collection& rhs);
  /// @brief Move assignment operator.
  Collection& operator=(Collection&& rhs) noexcept;
  /**
   * @brief Getter for a shared pointer.
   * @return std::shared_ptr<Collection> A shared pointer to this Collection.
   */
  std::shared_ptr<Collection> getPointer() {
    return this->shared_from_this();
  }
  /**
   * @brief Gets an object with the given ObjectClass and ID from this collection.
   *
   * @tparam ObjectClass The derived class to convert the object to.
   * @param id The ID to search for.
   * @throws IDNotFoundException Thrown if no object with the given ID is present.
   * @return ObjectClass The derived Object.
   */
  template<class ObjectClass>
  ObjectClass get(ID id);
  /**
   * @brief Finds the first object matching a query and returns its ID.
   *
   * If no matching object is found, returns boost::none.
   */
  boost::optional<ID> find(const std::string& query);
  /**
   * @brief Get one occurrence of a specific (type and filter) object, also applying an update.
   *
   * @tparam ObjectClass The database class to convert the object to.
   * @param filter The filter to apply when searching for the object.
   * @param update The update to apply to the found object.
   *
   * @throws ObjectTypeMismatchException If the object found (filter) does not
   *   match the given class type (template).
   *
   * @returns A linked object with populated ID. Returns an object without an
   *   ID and no linked collection if no object matching the filter was found.
   */
  template<class ObjectClass>
  ObjectClass getAndUpdateOne(const std::string& filter, const std::string& update = "", const std::string& sort = "");
  /**
   * @brief Get one occurrence of a specific (type and filter) object (useful for existence checks).
   *
   * @tparam ObjectClass The database class to convert the object to.
   * @param filter The filter to apply when searching for the object.
   * @throws ObjectTypeMismatchException If the object found (filter) does not
   *   match the given class type (template).
   * @returns A linked object instance populated with an ID. Returns an object
   *   without an ID and no linked collection if no object matching the filter
   *   was found.
   */
  template<class ObjectClass>
  ObjectClass getOne(const std::string& filter, const std::string& sort = "");
  /**
   * @brief Checks if an entry with the given ID is present in this collection.
   *
   * @param id The ID to be checked for.
   */
  bool has(const ID& id);
  /**
   * @brief Checks if an entry with the given ID and Object-type is present in this collection.
   *
   * For existence checks with a full query, use getOne().
   *
   * @tparam ObjectClass The class of object to be checked for.
   * @param id The ID to be checked for.
   */
  template<class ObjectClass>
  bool has(const ID& id);
  /**
   * @brief Queries for all Objects of a given type that match a given query.
   *
   * @tparam ObjectClass The class of object to be checked for.
   * @param selection The query, given as a json string.
   * @returns A vector of all Objects matching the query each populated with an
   *   ID and linked to this collection.
   */
  template<class ObjectClass>
  std::vector<ObjectClass> query(const std::string& selection);
  /**
   * @brief Picks randomly the given number of objects out of all Objects.
   *
   * @note if the given number is equal or larger than the number of objects in
   * the collection, the vector with all objects of the collection is returned,
   * so the size of the returned vector can never be larger than the number of
   * objects in the collection
   *
   * @tparam ObjectClass The class of object to be selected.
   * @param nSamples The number of objects to be selected
   * @return std::vector<ObjectClass> A vector of size nSamples
   *   each populated with an ID and linked to this collection.
   */
  template<class ObjectClass>
  std::vector<ObjectClass> randomSelect(std::int32_t nSamples);
  /**
   * @brief Queries for all objects that match a given query and returns
   *        their count.
   *
   * Note that this function is very performant if all of the queried fields
   * are indexed.
   *
   * @param selection The query, given as a json string.
   * @return unsigned int The number of documents matching the query.
   */
  unsigned count(const std::string& selection);
  /**
   * @brief A small helper to allow loops over documents in the database.
   *
   * For the usage see Collection::iteratorQuery().
   *
   * @tparam Obj The class of object to be iterated over.
   */
  template<class Obj>
  class CollectionLooper {
    /// @cond
    struct Impl;
    std::unique_ptr<Impl> _pImpl;

   public:
    CollectionLooper(Collection& coll, bsoncxx::v_noabi::document::view_or_value query);
    ~CollectionLooper();
    CollectionLooper(const CollectionLooper&);
    CollectionLooper(CollectionLooper&&) noexcept;
    CollectionLooper& operator=(const CollectionLooper&);
    CollectionLooper& operator=(CollectionLooper&&) noexcept;
    Obj operator*();
    CollectionLooper& operator++();
    CollectionLooper operator++(int /* dummy */);
    bool done() const;
    /// @endcond
  };

  /**
   * @brief Queries for all Objects of a given type that match a given query.
   *
   * This function only has one database object loaded at a time.\n
   * Usage:\n
    @code
    std::string query = "{ \"chemicalFormula\" : { \"$eq\" : \"mock\" } }";
    auto coll = db.getCollection("compounds");
    for (auto iter = coll.iteratorQuery<Compound>(query); !iter.done(); ++iter){
      Compound comp = *iter;
      foo(comp);
    }
    @endcode
   *
   * @tparam ObjectClass The class of object to be checked for.
   * @param selection The query, given as a json string.
   * @return CollectionLooper<ObjectClass> A helper to loop over all elements.
   */
  template<class ObjectClass>
  CollectionLooper<ObjectClass> iteratorQuery(const std::string& selection);
  /**
   * @brief Getter for the mongocxx collection class underlying this class.
   * @return const mongocxx::collection& The mongocxx collection class underlying this class.
   */
  mongocxx::v_noabi::collection& mongocxx();

 private:
  // convert cursor to vector
  template<class ObjectClass>
  std::vector<ObjectClass> toVector(mongocxx::v_noabi::cursor& cursor);

  std::unique_ptr<mongocxx::v_noabi::collection> _collection;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_COLLECTION_H_ */
