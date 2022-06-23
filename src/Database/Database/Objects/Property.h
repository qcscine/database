/**
 * @file Property.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_PROPERTY_H_
#define DATABASE_PROPERTY_H_

/* Internal Include */
#include "Database/Layout.h"
#include "Database/Objects/Object.h"
#include "boost/optional.hpp"

namespace Scine {
namespace Database {

class Model;
class Manager;
class Calculation;
class Structure;

/**
 * @class Property Property.h
 * @brief A base class for all properties of structures, stored in
 *        the database.
 *
 * Each property is a combination of some data,
 * a reference (ID) to a calculation that this data was generated from
 * and a reference to structure it is a property of.
 * Multiple properties can reference the same Calculation and Structure,
 * but only one Calculation and Structure can be referenced per Property.
 *
 * Developer notes:
 *  - All derived classes need to define a:
 *    @code static constexpr const char* propertytype @endcode
 */
class Property : public Object {
 public:
  /// @brief The name of this derived database object.
  static constexpr const char* objecttype = "property";

  // Inherit constructors
  using Object::Object;

  /**
   * @brief Get the derived property.
   * @throws ObjectTypeMismatchException Thrown if the object is not of the requested type.
   * @tparam PropertyClass The type of derived object to convert to.
   * @return PropertyClass The derived object.
   */
  template<class PropertyClass>
  PropertyClass getDerived() const;
  /**
   * @brief Checks if the derived class type of this Property matches the template argument.
   *
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   *
   * @tparam PropertyClass The class type to be check for.
   */
  template<class PropertyClass>
  bool isOfType() const;
  /**
   * @brief Get the property name.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::string The current property name.
   */
  std::string getPropertyName() const;
  /**
   * @brief Set the property name.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param name The property name.
   */
  void setPropertyName(const std::string& name) const;
  /**
   * @brief Get the Model.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return Model The Model of this calculation.
   */
  Model getModel() const;
  /**
   * @brief Set the Model.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param model The new model for this calculation.
   */
  void setModel(const Model& model) const;
  /*! @brief Get the related structure-id, if present
   * @throws MissingLinkedCollectionException If hasLink is false
   * @throws MissingIDException If hasId is false
   * @returns The structure ID if set, otherwise None
   */
  boost::optional<ID> partialGetStructureId() const;
  /**
   * @brief Get the related structure-id.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the object can not be found or if is has no related ID set.
   * @return ID The ID.
   */
  ID getStructure() const;
  //! Fetch the related structure
  Structure getStructure(const Manager& manager, const std::string& collection = Layout::DefaultCollection::structure) const;
  /**
   * @brief Checks if a Structure is linked to this property.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  bool hasStructure() const;
  /**
   * @brief Set the structure-id.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The new ID.
   */
  void setStructure(const ID& id) const;
  /**
   * @brief Removes any structure-id currently present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearStructure() const;
  /**
   * @brief Get the related calculation-id.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the object can not be found or if is has no related ID set.
   * @return ID The ID.
   */
  ID getCalculation() const;
  //! Fetch the related calculation
  Calculation getCalculation(const Manager& manager, const std::string& collection = Layout::DefaultCollection::calculation) const;
  /**
   * @brief Checks if a Calculation is linked to this property.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  bool hasCalculation() const;
  /**
   * @brief Set the calculation-id.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The new ID.
   */
  void setCalculation(const ID& id) const;
  /**
   * @brief Removes any calculation-id currently present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearCalculation() const;
  /**
   * @brief Get the comment.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::string The comment.
   */
  std::string getComment() const;
  /**
   * @brief Set the comment.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param comment The comment.
   */
  void setComment(const std::string& comment) const;
  /**
   * @brief Checks if a comment is present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  bool hasComment() const;
  /**
   * @brief Removes any comment currently present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearComment() const;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_PROPERTY_H_ */
