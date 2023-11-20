/**
 * @file VectorProperty.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_VECTORPROPERTY_H_
#define DATABASE_VECTORPROPERTY_H_

/* Internal Include */
#include <Database/Objects/Property.h>
/* External Includes */
#include <Eigen/Dense>

namespace Scine {
namespace Database {

/**
 * @class VectorProperty
 * @brief A property class for data stored in a vector.
 */
class VectorProperty : public Property {
 public:
  /// @brief The name of this derived database object.
  static constexpr const char* objecttype = "property";
  /// @brief The type of property.
  static constexpr const char* propertytype = "vector_property";
  // Inherit constructors
  using Property::Property;

  /* @brief Create a new property in the database
   *
   * @param collection The collection to add the property to
   * @param model      The model used to calculate the property.
   * @param name       The name of the property (e.g. electronic_energy).
   * @param data       The initial data.
   *
   * @throws MissingLinkedCollectionException if the collection pointer is empty.
   * @returns An instance of the new property
   */
  static VectorProperty create(const std::string& name, const Model& model, const Eigen::VectorXd& data,
                               const CollectionPtr& collection);

  /* @brief Create a new property in the database
   *
   * @param collection  The collection to add the property to
   * @param model       The model used to calculate the property.
   * @param name        The name of the property (e.g. electronic_energy).
   * @param structure   The structure this property was generated for
   * @param calculation The calculation this property was generated with
   * @param data        The initial data.
   *
   * @throws MissingLinkedCollectionException if the collection pointer is empty.
   * @returns An instance of the new property
   */
  static VectorProperty create(const std::string& name, const Model& model, const Eigen::VectorXd& data,
                               const ID& structure, const ID& calculation, const CollectionPtr& collection);

  /**
   * @brief Creates a new document in the linked collection.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param model        The model used to calculate the property.
   * @param propertyName The name of the property (e.g. electronic_energy).
   * @param data         The inital data.
   * @return ID          The id of the inserted document.
   */
  ID create(const Model& model, const std::string& propertyName, const Eigen::VectorXd& data);
  /**
   * @brief Creates a new document in the linked collection.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param model        The model used to calculate the property.
   * @param propertyName The name of the property (e.g. electronic_energy).
   * @param structure    The structure this property was generated for.
   * @param calculation  The calculation this property was generated with.
   * @param data         The inital data.
   * @return ID          The id of the inserted document.
   */
  ID create(const Model& model, const std::string& propertyName, const ID& structure, const ID& calculation,
            const Eigen::VectorXd& data);
  /**
   * @brief Get the data.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return Eigen::VectorXd The data.
   */
  Eigen::VectorXd getData() const;
  /**
   * @brief Set the data.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param data The new data
   */
  void setData(const Eigen::VectorXd& data) const;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_VECTORPROPERTY_H_ */
