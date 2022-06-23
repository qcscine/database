/**
 * @file Structure.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_STRUCTURE_H_
#define DATABASE_STRUCTURE_H_

/* Internal Includes */
#include "Database/Layout.h"
#include "Database/Objects/Object.h"
/* External Includes */
#include <Utils/Geometry/AtomCollection.h>
#include <map>

namespace Scine {

namespace Utils {
class AtomCollection;
}

namespace Database {

class Collection;
class Model;

/**
 * @class Structure Structure.h
 * @brief A molecular structure representation for the database.
 *
 * A structure represents one Geometry with a specific charge and
 * multiplicity (spin), hence it represents one particular
 * point on one particular potential energy surface.
 */
class Structure : public Object {
 public:
  /// @brief The name of this derived database object.
  static constexpr const char* objecttype = "structure";
  // Inherit constructors
  using Object::Object;
  /**
   * @brief The set of all labels possible for a structure.
   */
  using LABEL = StructureLabel;
  /**
   * @brief Creates a new Structure in the remote database.
   *
   * The label is defaulted to NONE. The model is defaulted to be the model
   * representing a human guess.
   *
   * @param atoms         The atoms of the structure.
   * @param charge       The overall charge (defines the electron count).
   * @param multiplicity The spin multiplicity for the set of electrons.
   * @param collection   The database collection to write this object into
   *
   * @throws MissingCollectionException Thrown if the collection pointer is empty
   * @return ID          The ID of the newly inserted structure.
   */
  static Structure create(const Utils::AtomCollection& atoms, int charge, int multiplicity, const CollectionPtr& collection);
  /**
   * @brief Creates a new Structure in the remote database.
   *
   * The label is defaulted to NONE. The model is defaulted to be the model
   * representing a human guess.
   *
   * @param atoms         The atoms of the structure.
   * @param charge       The overall charge (defines the electron count).
   * @param multiplicity The spin multiplicity for the set of electrons.
   * @param model        The model used to determine this geometry.
   * @param label        The label for the structure, see Structure::LABEL for a complete list of options.
   * @param collection   The database collection to write this object into
   *
   * @throws MissingCollectionException Thrown if the collection pointer is empty
   * @return ID          The ID of the newly inserted structure.
   */
  static Structure create(const Utils::AtomCollection& atoms, int charge, int multiplicity, const Model& model,
                          LABEL label, const CollectionPtr& collection);
  /**
   * @brief Creates a new Structure in the remote database.
   *
   * The label is defaulted to NONE.
   * The model is defaulted to be the model representing a
   * human guess.
   *
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param atoms         The atoms of the structure.
   * @param charge       The overall charge (defines the electron count).
   * @param multiplicity The spin multiplicity for the set of electrons.
   * @return ID          The ID of the newly inserted structure.
   */
  ID create(const Utils::AtomCollection& atoms, int charge, int multiplicity);
  /**
   * @brief Creates a new Structure in the remote database.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param atoms         The atoms of the structure.
   * @param charge       The overall charge (defines the electron count).
   * @param multiplicity The spin multiplicity for the set of electrons.
   * @param model        The model used to determine this geometry.
   * @param label        The label for the structure, see Structure::LABEL for a complete list of options.
   * @return ID          The ID of the newly inserted structure.
   */
  ID create(const Utils::AtomCollection& atoms, int charge, int multiplicity, const Model& model, LABEL label);
  /**
   * @brief Getter for all atoms.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return Utils::AtomCollection The currently stored atoms.
   */
  Utils::AtomCollection getAtoms() const;
  /**
   * @brief Replaces the current atoms with new ones.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param atoms The new atoms.
   */
  void setAtoms(const Utils::AtomCollection& atoms) const;
  /**
   * @brief Checks if the structures has atoms associated and gives their amount.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The number of atoms associated with the structure.
   */
  int hasAtoms() const;
  /**
   * @brief Removes all atoms currently present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearAtoms() const;
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
  /**
   * @brief Get the label.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return STATUS The current label.
   */
  LABEL getLabel() const;
  /**
   * @brief Set the label.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param label The new label.
   */
  void setLabel(LABEL label) const;
  /**
   * @brief Get the molecular charge.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The molecular charge.
   */
  int getCharge() const;
  /**
   * @brief Set the molecular charge.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param charge The new molecular charge.
   */
  void setCharge(int charge) const;
  /**
   * @brief Get the total electronic spin as multiplicity.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The electronic spin multiplicity.
   */
  int getMultiplicity() const;
  /**
   * @brief Set the electronic spin.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param multiplicity The new multiplicity.
   */
  void setMultiplicity(int multiplicity) const;
  /**
   * @brief Get linked compound-id
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the structure does not have a linked compound ID.
   * @return ID The ID of the linked compound.
   */
  ID getCompound() const;
  /**
   * @brief Links the structure to a compound.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The new ID to link.
   */
  void setCompound(const ID& id) const;
  /**
   * @brief Checks if the structure is linked to a compound.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true  If the structure is linked to a compound.
   * @return false If the structure is not linked to a compound.
   */
  bool hasCompound() const;
  /**
   * @brief Removes the current link to compound.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearCompound() const;
  /**
   * @brief Checks if a single property is given under the specified key.
   *
   * This function is part of the one vs. many split in accessing properties
   * in a given Structure.
   * In general all properties with the same key (but different model)
   * are stored in the same list.
   * In the special case of a property that is model independent
   * and thus only one of version of the property exists, this function and
   * others expecting a single property and working on a single
   * property can be used.
   *
   * @param key The property key.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true  If exactly one property is registered under the given key
   * @return false If exactly none or more than one property is registered under the given key
   */
  bool hasProperty(const std::string& key) const;
  /**
   * @brief Checks if a specific property (id) is listed in this structure.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID to check for.
   * @return true  If the given ID is present in the structure.
   * @return false If the given ID is not present in the structure.
   */
  bool hasProperty(const ID& id) const;
  /**
   * @brief Get a single property.
   *
   * This function is part of the one vs. many split in accessing properties
   * in a given Structure.
   * In general all properties with the same key (but different model)
   * are stored in the same list.
   * In the special case of a property that is model independent
   * and thus only one of version of the property exists, this function and
   * others expecting a single property and working on a single
   * property can be used.
   *
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws FieldException Thrown if the property-list has more than one entry.
   * @param key The key of the property.
   * @return ID The property's ID.
   */
  ID getProperty(const std::string& key) const;
  /**
   * @brief Set/reset the property-list to a single one for a given key.
   *
   * This function is part of the one vs. many split in accessing properties
   * in a given Structure.
   * In general all properties with the same key (but different model)
   * are stored in the same list.
   * In the special case of a property that is model independent
   * and thus only one of version of the property exists, this function and
   * others expecting a single property and working on a single
   * property can be used.
   *
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the property.
   * @param id  The new property-id.
   */
  void setProperty(const std::string& key, const ID& id) const;
  /**
   * @brief Add a single property to a given key.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the property.
   * @param id  The ID of the property to add.
   */
  void addProperty(const std::string& key, const ID& id) const;
  /**
   * @brief Removes a single property from a given key.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the property.
   * @param id  The ID of the property to remove.
   */
  void removeProperty(const std::string& key, const ID& id) const;
  /**
   * @brief Set/replace all properties of a given key.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the properties.
   * @param ids A vector of IDs.
   */
  void setProperties(const std::string& key, const std::vector<ID>& ids) const;
  /**
   * @brief Get all properties stored under one key.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key to be used.
   * @return std::vector<ID> The list of properties.
   */
  std::vector<ID> getProperties(const std::string& key) const;
  /**
   * @brief Returns all properties fitting a given model query that are
   *        presently stored under the given key.
   *
   * All fields of the given model that are set to "any" will be ignored
   * in the query. Thus all occurrences will be included in the returned
   * vector.
   *
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key              The key to check.
   * @param model            The model query.
   * @param collection       The collection to find the properties in.
   * @return std::vector<ID> The resulting list of properties.
   */
  std::vector<ID> queryProperties(const std::string& key, const Model& model, std::shared_ptr<Collection> collection) const;
  /**
   * @brief Checks if properties are present under a given key.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key to check the number of properties for.
   * @return int The number of properties stored under the given key.
   */
  int hasProperties(const std::string& key) const;
  /**
   * @brief Clears all properties stored under the given key.
   *
   * Also removes the key.
   *
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key to remove including all its linked properties.
   */
  void clearProperties(const std::string& key) const;
  /**
   * @brief Get all stores properties as a complete map.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::map<std::string, std::vector<ID>> The map of all properties.
   */
  std::map<std::string, std::vector<ID>> getAllProperties() const;
  /**
   * @brief Set/Replace all properties.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param properties The new mao of properties.
   */
  void setAllProperties(const std::map<std::string, std::vector<ID>>& properties) const;
  /**
   * @brief Removes all stored properties.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearAllProperties() const;
  /**
   * @brief Get a graph representation of the structure.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the graph representation.
   * @return std::string The graph representation of the structure.
   */
  std::string getGraph(const std::string& key) const;
  /**
   * @brief Check if a graph representation with the given key is stored.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the graph representation.
   * @return true If a graph representation of the structure with the given label exists.
   * @return false If no graph representation of the structure with the given label exists.
   */
  bool hasGraph(const std::string& key) const;
  /**
   * @brief Set the graph representation of the structure.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key   The key of the graph representation, e.g. "SMILES".
   * @param graph The graph representation of the structure.
   */
  void setGraph(const std::string& key, const std::string& graph) const;
  /**
   * @brief Removes a single graph representation.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key   The key of the graph representation, e.g. "SMILES".
   */
  void removeGraph(const std::string& key) const;
  /**
   * @brief Get all graph representations of the structure.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::map<std::string, std::string> All graph representations of the structure.
   */
  std::map<std::string, std::string> getGraphs() const;
  /**
   * @brief Set all graph representations of the structure.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param graphs A map of different graph representations.
   */
  void setGraphs(const std::map<std::string, std::string>& graphs) const;
  /**
   * @brief Checks if a graph representation of the structure is present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The number of graph representation currently held.
   */
  int hasGraphs() const;
  /**
   * @brief Removes any graph representation of the structure currently present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearGraphs() const;
  /**
   * @brief Gets the stored comment
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::string The graph representation of the structure.
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
   * @return true  If a comment is present.
   * @return false If no comment is present.
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

#endif /* DATABASE_STRUCTURE_H_ */
