/**
 * @file Flask.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_FLASK_H_
#define DATABASE_FLASK_H_

/* Internal Includes */
#include "Database/Layout.h"
#include "Database/Objects/Object.h"
/* External Includes */
#include <Utils/Typenames.h>

namespace Scine {
namespace Database {

class Manager;
class Structure;
class Compound;
class Reaction;

/**
 * @class Flask Flask.h
 * @brief A group of molecular structures, all complexes of multiple molecules.
 *
 * All structures in one Flask each represent a complex of the same set of molecules.
 * A set of molecular structures with the same nuclear composition and connectivity
 * in terms of chemical bonds.
 * Its connection to other flasks and compounds via reactions should span the
 * main view of any network.
 *
 * Developer notes:
 *  - Flasks, as do all other meta Objects, only store IDs of other Objects, they
 *    do not store the objects themselves. This keeps the storage requirement minimal
 *    and does not allow for unused copies of Objects and their data in these meta
 *    type Objects.
 */
class Flask : public Object {
 public:
  /// @brief The name of this derived database object.
  static constexpr const char* objecttype = "flask";

  // Inherit constructors
  using Object::Object;

  /**
   * @brief Create a new Flask in the remote database.
   *
   * @param collection The collection to generate the Flask in
   * @param structures The initial list of Structures in the Flask.
   * @param compounds  The list of Compounds that are combined in the Flask.
   * @param explorationDisabled Disable this Flask for further explorations.
   *
   * @throws MissingLinkedCollectionException If @p collection does not hold a collection
   * @returns The new Flask instance.
   */
  static Flask create(const std::vector<ID>& structures, const std::vector<ID>& compounds,
                      const CollectionPtr& collection, bool explorationDisabled = false);
  /**
   * @brief Creates a new Flask in the remote database.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param structures The initial list of Structures in the Flask.
   * @param compounds  The list of Compounds that are combined in the Flask.
   * @param explorationDisabled Disable this Flask for further explorations.
   * @return ID        The ID of the newly inserted Flask.
   */
  ID create(const std::vector<ID>& structures, const std::vector<ID>& compounds, bool explorationDisabled = false);
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
   * @brief Check if the Flask is part of a given reaction (ID).
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Reaction to be checked for.
   * @return true  If the Flask is part of the given Reaction (ID).
   * @return false If the Flask is not part of the given Reaction (ID).
   */
  bool hasReaction(const ID& id) const;
  /**
   * @brief Add a Reaction to the list of known reactions of this Flask.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Reaction to be added.
   */
  void addReaction(const ID& id) const;
  /**
   * @brief Removes a Reaction from the list of known reactions of this Flask.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Reaction to be removed.
   */
  void removeReaction(const ID& id) const;
  /**
   * @brief Checks for the amount of known reactions of the Flask.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The number of reactions linked to the Flask.
   */
  int hasReactions() const;
  /**
   * @brief Get all known reactions.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::vector<ID> The vector of all reaction IDs.
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
   * @brief Check if the Flask holds the queried ID as a Structure.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Structure to be checked for.
   * @return true  If the Structure ID is part of the Flask.
   * @return false If the Structure ID is not part of the Flask.
   */
  bool hasStructure(const ID& id) const;
  /**
   * @brief Add a Structure to the list of structures in this Flask.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Structure to be added.
   */
  void addStructure(const ID& id) const;
  /**
   * @brief Removes a structure from this Flask.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Structure to be removed.
   */
  void removeStructure(const ID& id) const;
  /**
   * @brief Checks for the amount of structures in this Flask.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The number of structures linked to the Flask.
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
  /**
   * @brief Check if the Flask holds the queried ID as a Compound.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Compound to be checked for.
   * @return true  If the Compound ID is part of the Flask.
   * @return false If the Compound ID is not part of the Flask.
   */
  bool hasCompound(const ID& id) const;
  /**
   * @brief Checks for the number of Compounds in this Flask.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The number of Compounds linked to the Flask.
   */
  int hasCompounds() const;
  /**
   * @brief Get the all stored Compounds.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::vector<ID> The vector of all Compound IDs.
   */
  std::vector<ID> getCompounds() const;
  //! Fetch all stored structures
  std::vector<Compound> getCompounds(const Manager& manager,
                                     const std::string& collection = Layout::DefaultCollection::compound) const;
  /**
   * @brief Set/replace all Compounds.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param ids The new Compounds.
   */
  void setCompounds(const std::vector<ID>& ids) const;
  /**
   * @brief Removes all linked Compounds.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearCompounds() const;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_FLASK_H_ */
