/**
 * @file Compound.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_COMPOUND_H_
#define DATABASE_COMPOUND_H_

/* Internal Includes */
#include "Database/Layout.h"
#include "Database/Objects/Object.h"
/* External Includes */
#include <Utils/Typenames.h>

namespace Scine {
namespace Database {

class Manager;
class Structure;
class Reaction;

/**
 * @class Compound Compound.h
 * @brief A group of molecular structures, all (relative) minima on their respective
 *        potential energy surface.
 *
 * All structures in one Compound represent the same molecule.
 * A set of molecular structures with the same nuclear composition and connectivity
 * in terms of chemical bonds.
 * A compound thus represents a chemical compound in a general sense.
 * Its connection to other compounds via reactions should span the main view of any
 * network.
 *
 * Developer notes:
 *  - Compounds, as do all other meta Objects, only store IDs of other Objects, they
 *    do not store the objects themselves. This keeps the storage requirement minimal
 *    and does not allow for unused copies of Objects and their data in these meta
 *    type Objects.
 */
class Compound : public Object {
 public:
  /// @brief The name of this derived database object.
  static constexpr const char* objecttype = "compound";

  // Inherit constructors
  using Object::Object;

  /**
   * @brief Create a new Compound in the remote database.
   *
   * @param collection The collection to generate the compound in
   * @param structures The initial list of structures in the compound.
   *
   * @throws MissingLinkedCollectionException If @p collection does not hold a collection
   * @returns the new instance
   */
  static Compound create(const std::vector<ID>& structures, const CollectionPtr& collection);
  /**
   * @brief Creates a new Compound in the remote database.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param structures The initial list of structures in the compound.
   * @return ID        The ID of the newly inserted structure.
   */
  ID create(const std::vector<ID>& structures);
  /**
   * @brief Get the centroid structure.
   *
   * Note, the centroid is the first entry in the vector of structures
   * that is the base object of this class.
   * Thus changing the centroid can be achieved using standard operations
   * on the vector of structures.
   *
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return ID The central Structure ID.
   */
  ID getCentroid() const;
  //! Fetch linked centroid structure
  Structure getCentroid(const Manager& manager, const std::string& collection = Layout::DefaultCollection::structure) const;
  /**
   * @brief Check if the compound is part of a given reaction (ID).
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Reaction to be checked for.
   * @return true  If the compound is part of the given Reaction (ID).
   * @return false If the compound is not part of the given Reaction (ID).
   */
  bool hasReaction(const ID& id) const;
  /**
   * @brief Add a Reaction to the list of known reaction of this compound.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Reaction to be added.
   */
  void addReaction(const ID& id) const;
  /**
   * @brief Removes a Reaction from the list of known reaction of this compound.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Reaction to be removed.
   */
  void removeReaction(const ID& id) const;
  /**
   * @brief Checks for the amount of known reaction of the Compound.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The number of reactions linked to the Compound.
   */
  int hasReactions() const;
  /**
   * @brief Get all known reactions.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::vector<ID> The vector of all reaction ids.
   */
  std::vector<ID> getReactions() const;
  //! Fetch linked reactions
  std::vector<Reaction> getReactions(const Manager& manager,
                                     const std::string& collection = Layout::DefaultCollection::reaction) const;
  /**
   * @brief Set/replace all known reactions.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param ids The new set of reaction ids.
   */
  void setReactions(const std::vector<ID>& ids) const;
  /**
   * @brief Removes all linked Reactions.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearReactions() const;
  /**
   * @brief Check if the Compound holds the queried ID as a Structure.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Structure to be checked for.
   * @return true  If the Structure ID is part of the Compound.
   * @return false If the Structure ID is not part of the Compound.
   */
  bool hasStructure(const ID& id) const;
  /**
   * @brief Add a Structure to the list of structures in this compound.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Structure to be added.
   */
  void addStructure(const ID& id) const;
  /**
   * @brief Removes a structure from this compound.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Structure to be removed.
   */
  void removeStructure(const ID& id) const;
  /**
   * @brief Checks for the amount of structures in this Compound.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The number of structures linked to the Compound.
   */
  int hasStructures() const;
  /**
   * @brief Get the all stored structures.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::vector<ID> The vector of all structure ids.
   */
  std::vector<ID> getStructures() const;
  //! Fetch all stored structures
  std::vector<Structure> getStructures(const Manager& manager,
                                       const std::string& collection = Layout::DefaultCollection::structure) const;
  /**
   * @brief Set/replace all structures.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param ids The new structures.
   */
  void setStructures(const std::vector<ID>& ids) const;
  /**
   * @brief Removes all linked structures.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearStructures() const;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_COMPOUND_H_ */
