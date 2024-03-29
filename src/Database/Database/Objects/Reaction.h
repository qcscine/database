/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_REACTION_H_
#define DATABASE_REACTION_H_

/* Internal Include */
#include "Database/Layout.h"
#include "Database/Objects/Object.h"
#include "Database/Objects/ReactionEnums.h"
/* External Includes */
#include <vector>

namespace Scine {
namespace Database {

class Manager;
class ElementaryStep;

/**
 * @class Reaction Reaction.h
 * @brief A reaction is a set of elementary steps, all connecting structures from
 *        the same compounds in the same way.
 *
 * A Reaction as defined in this class is reaction of Compounds or Flasks, resulting in
 * other Compounds or Flasks, this Reaction can then be expressed as a set of ElementarySteps
 * connecting Structures of the aforementioned Compounds/Flasks.
 * The reactants (Compound/Flask) are held as pair of vectors, describing both endpoints
 * of the Reaction.
 */
class Reaction : public Object {
 public:
  /// @brief The name of this derived database object.
  static constexpr const char* objecttype = "reaction";

  // Inherit constructors
  using Object::Object;

  /**
   * @brief Creates a new reaction in the remote database.
   *
   * @param lhs The list of compounds on the left hand side of the reaction.
   * @param rhs The list of compounds on the right hand side of the reaction.
   * @param collection The collection to create the reaction in
   * @param lhsTypes The list of identifiers indicating if the reagent on the
   *                 left hand side is a Flask or Compound. Empty lists default
   *                 to all reagents on that side being Compounds.
   * @param rhsTypes The list of identifiers indicating if the reagent on the
   *                 right hand side is a Flask or Compound. Empty lists default
   *                 to all reagents on that side being Compounds.
   *
   * @throws MissingCollectionException Thrown if collection pointer is empty
   * @returns an instance of the new Reaction
   */
  static Reaction create(const std::vector<ID>& lhs, const std::vector<ID>& rhs, const CollectionPtr& collection,
                         const std::vector<COMPOUND_OR_FLASK>& lhsTypes = {},
                         const std::vector<COMPOUND_OR_FLASK>& rhsTypes = {});
  /**
   * @brief Creates a new reaction in the remote database.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param lhs The list of compounds on the left hand side of the reaction.
   * @param rhs The list of compounds on the right hand side of the reaction.
   * @param lhsTypes The list of identifiers indicating if the reagent on the
   *                 left hand side is a Flask or Compound. Empty lists default
   *                 to all reagents on that side being Compounds.
   * @param rhsTypes The list of identifiers indicating if the reagent on the
   *                 right hand side is a Flask or Compound. Empty lists default
   *                 to all reagents on that side being Compounds.
   * @return ID The ID of the newly inserted reaction.
   */
  ID create(const std::vector<ID>& lhs, const std::vector<ID>& rhs, const std::vector<COMPOUND_OR_FLASK>& lhsTypes = {},
            const std::vector<COMPOUND_OR_FLASK>& rhsTypes = {});
  /**
   * @brief Check if the elementary step (ID) is part of the Reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the elementary step to be checked for.
   */
  bool hasElementaryStep(const ID& id) const;
  /**
   * @brief Adds an elementary step to reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the elementary step to be added.
   */
  void addElementaryStep(const ID& id) const;
  /**
   * @brief Removes an elementary step from the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the elementary step to be removed.
   */
  void removeElementaryStep(const ID& id) const;
  /**
   * @brief Checks for the amount of elementary steps in this reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The number of elementary steps in the reaction.
   */
  int hasElementarySteps() const;
  /**
   * @brief Set/replace all elementary steps.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param ids The new elementary steps.
   */
  void setElementarySteps(const std::vector<ID>& ids) const;
  /**
   * @brief Get the all stored elementary steps.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::vector<ID> The vector of all elementary step ids.
   */
  std::vector<ID> getElementarySteps() const;
  //! Fetch all stored elementary steps
  std::vector<ElementaryStep>
  getElementarySteps(const Manager& manager, const std::string& collection = Layout::DefaultCollection::elementaryStep) const;
  /**
   * @brief Removes all elementary steps linked to the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearElementarySteps() const;
  /**
   * @brief Checks if a compound is part of the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id    The ID to be checked for.
   * @return SIDE The side of the reaction on which the ID was found.
   */
  SIDE hasReactant(const ID& id) const;
  /**
   * @brief Add a single reactant (Compound/Flask) to one or both sides of the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id   The reactant ID.
   * @param side The side(s) on which to work.
   * @param type The type of the reactant.
   */
  void addReactant(const ID& id, SIDE side, COMPOUND_OR_FLASK type = COMPOUND_OR_FLASK::COMPOUND) const;
  /**
   * @brief Remove a single reactant (Compound/Flask) from one or both sides of the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id   The compound id.
   * @param side The side(s) on which to work.
   */
  void removeReactant(const ID& id, SIDE side) const;
  /**
   * @brief Count the number of reactants on both sides of the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::tuple<int, int> The number of reactants on the sides of the reaction.
   */
  std::tuple<int, int> hasReactants() const;
  /**
   * @brief Set the Reactants
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param ids  The new reactant IDs.
   * @param side The side of the reaction on which to replace the reactant-ids.
   * @param types The types of the reactant. An empty list defaults to all-compounds.
   */
  void setReactants(const std::vector<ID>& ids, SIDE side, const std::vector<COMPOUND_OR_FLASK>& types = {}) const;
  /**
   * @brief Get the Reactants
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param side The sides of which the ids should be returned.
   * @return std::tuple<std::vector<ID>, std::vector<ID>> The stored reactants.
   *         If the only one of the sides was requested, the other part of the
   *         tuple will an empty vector.
   */
  std::tuple<std::vector<ID>, std::vector<ID>> getReactants(SIDE side) const;
  /**
   * @brief Get the Reactant types
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param side The sides of which the types should be returned.
   * @return std::tuple<std::vector<COMPOUND_OR_FLASK>, std::vector<COMPOUND_OR_FLASK>>
   *         The stored reactant types.
   *         If the only one of the sides was requested, the other part of the
   *         tuple will an empty vector.
   */
  std::tuple<std::vector<COMPOUND_OR_FLASK>, std::vector<COMPOUND_OR_FLASK>> getReactantTypes(SIDE side) const;
  /**
   * @brief Get the Reactant type
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the reactant ID is not present in the object.
   * @param id The new reactant ID.
   * @return COMPOUND_OR_FLASK The type of the checked reactant.
   */
  COMPOUND_OR_FLASK getReactantType(const ID& id) const;
  /**
   * @brief Removes the reactants of one or both sides of the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param side The side(s) of the reaction the clear the reactant of.
   */
  void clearReactants(SIDE side) const;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_REACTION_H_ */
