/**
 * @file Object.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_OBJECT_H_
#define DATABASE_OBJECT_H_

/* Internal Include */
#include <Database/Id.h>
/* External Includes */
#include "boost/optional/optional_fwd.hpp"
#include <chrono>
#include <memory>

namespace bsoncxx {
inline namespace v_noabi {
namespace document {
class value;
} // namespace document
} // namespace v_noabi
} // namespace bsoncxx

namespace Scine {
namespace Database {

class Collection;

/**
 * @class Object Object.h
 * @brief The base class for any object stored in a database.
 *
 * The goal of this object is to abstract the actual interaction with the database.
 * All of the different types of objects will interact with the database and its
 * collections in a standardized way.
 * This standard is defined here.
 *
 * @note
 * For the generation of derived classes and the idea for this interface:
 *  - All derived classes need to define a:
 *    @code static constexpr const char* objecttype = "something"; @endcode
 *    member.
 *  - All objects work on a push/pull basis, meaning all data held in memory
 *    at runtime has to be updated to or from the database via explicit calls to
 *    addto()/updatein() or update(). As a result, the data held at runtime
 *    does not require restrictions in terms of the access.
 *  - All data objects in the classes should be public members or public base
 *    classes, hence no additional member functions modifying the data should be
 *    needed.
 *  - Additional member functions checking presence of data should require
 *    a collection and should query only the remote collection given as an
 *    argument.
 *    The query of local data can be done via the public members and
 *    should not require additional functions.
 */
class Object {
 public:
  using CollectionPtr = std::shared_ptr<Collection>;

  /**
   * @brief Construct a new Object.
   *
   * Explicit to avoid unnecessary instances without any ID present.
   */
  explicit Object();
  /**
   * @brief Construct a new Object.
   * @param id The id of the object.
   */
  Object(const ID& id);
  /**
   * @brief Construct a new linked Object.
   * @param id The id of the object.
   * @param collection The collection to be linked to.
   */
  Object(const ID& id, CollectionPtr collection);
  /**
   * @brief Copy constructor.
   * @param obj The other Object.
   */
  Object(const Object& other);
  /**
   * @brief Assignment operator.
   * @param obj The other object, the RHS.
   * @returns Object& The resulting copy.
   */
  Object& operator=(const Object& obj);
  /// @brief Virtual default destructor.
  virtual ~Object() = default;
  /**
   * @brief Links the object to a collection.
   *
   * All calls to other functions will then try to edit/create
   * a document in the linked collection with the given ID.
   *
   * @param collection The collection to be linked to.
   */
  void link(CollectionPtr collection);
  /**
   * @brief Removes the link to the collection if there is one.
   */
  void detach();
  /**
   * @brief Getter for the linked collection if there is one.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @return std::shared_ptr<Collection> The linked collection.
   */
  std::shared_ptr<Collection> collection() const;
  /**
   * @brief Checks if the object is linked to a collection.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @return true  If the object is linked to a collection.
   * @return false If the object is not linked to a collection.
   */
  bool hasLink() const;
  /**
   * @brief Checks if the object exists in the linked collection.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @return true  If the object exists in the linked collection.
   * @return false If the object does not exist in the linked collection.
   */
  bool exists() const;
  /**
   * @brief Getter for the ID.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return const ID& A reference to the ID.
   */
  const ID& id() const;
  /**
   * @brief Checks if the Object has an ID.
   * @return true  If the Object has an ID.
   * @return false If the Object does not have an ID.
   */
  bool hasId() const {
    return static_cast<bool>(_id);
  }
  /**
   * @brief Get the raw content in its binary form.
   * @return bsoncxx::document::view_or_value The content of the object in
   *                                          BSON binary form.
   */
  bsoncxx::v_noabi::document::value getRawContent() const;
  /**
   * @brief Prints a JSON string of the Objects contents.
   */
  void print() const;
  /**
   * @brief Returns a JSON string of the Objects contents.
   * @returns std::string Returns the content as a json string.
   */
  std::string json() const;
  /**
   * @brief Removes the object from the linked collection.
   *
   * This will also remove the ID from the object, if one is present.
   *
   * @param expectPresence If true, expects the object to be present in the first place
   *                       and throws an error if it is not.
   */
  void wipe(bool expectPresence = false);
  /**
   * @brief Getter for the time of creation in the database.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingTimestampException if the timestamp is missing.
   * @return std::chrono::system_clock::time_point The time of creation in the database.
   */
  std::chrono::system_clock::time_point created() const;
  /**
   * @brief Getter for the last time of modification in the database.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingTimestampException if the timestamp is missing.
   * @return std::chrono::system_clock::time_point The last time of modification in the database.
   */
  std::chrono::system_clock::time_point lastModified() const;
  /**
   * @brief Checks if the timestamp is present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true  If the timestamp is present.
   * @return false If the timestamp is not present.
   */
  bool hasCreatedTimestamp() const;
  /**
   * @brief Checks if the timestamp is present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true  If the timestamp is present.
   * @return false If the timestamp is not present.
   */
  bool hasLastModifiedTimestamp() const;
  /**
   * @brief Updates or sets the timestap.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void touch() const;
  /**
   * @brief Compares the database timestamps of the objects.
   *
   * @throws MissingTimestampException if timestamps are missing.
   *
   * @param other        The other object to be compared;
   * @param modification If true compares the last modification instead of the creation timestamps.
   * @return true        If the rhs (other) object is younger.
   * @return false       If the the lhs objects is younger.
   */
  bool olderThan(const Object& other, bool modification = false) const;
  /**
   * @brief Enable all further analysis of the object.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void enable_analysis();
  /**
   * @brief Enable further explorations starting from the object.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void enable_exploration();
  /**
   * @brief Disable all further analysis of the object.
   *
   * This is equivalent to deleting the object with the difference that the
   * object will not be rediscovered over and over again.
   *
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void disable_analysis();
  /**
   * @brief Disable all further exploration starting from the object.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void disable_exploration();
  /**
   * @brief Checks the object is to be analyzed.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true  If a analysis is intended.
   * @return false If no analysis is intended.
   */
  bool analyze();
  /**
   * @brief Checks if the object is to be explored further, or to be used as
   *        a starting point for other explorations.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true  If further explorations are intended.
   * @return false If no further explorations are intended.
   */
  bool explore();

 protected:
  /// @brief The linked collection.
  std::shared_ptr<Collection> _collection;
  /// @brief The unique database ID of the object.
  std::unique_ptr<ID> _id;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_OBJECT_H_ */
