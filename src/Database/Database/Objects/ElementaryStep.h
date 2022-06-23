/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_ELEMENTARYSTEP_H_
#define DATABASE_ELEMENTARYSTEP_H_

/* Internal Include */
#include <Database/Layout.h>
#include <Database/Objects/Object.h>
/* External Includes */
#include <vector>

namespace Scine {

namespace Utils {
namespace BSplines {
class TrajectorySpline;
}
} // namespace Utils

namespace Database {

enum class SIDE;
class Manager;
class Reaction;
class Structure;

/**
 * @class ElementaryStep
 * @brief A path connecting two sets of structures.
 *
 * An ElementaryStep describes the reaction between two distinct sets
 * of Structures (not Compounds).
 *
 */
class ElementaryStep : public Object {
 public:
  /// @brief The name of this derived database object.
  static constexpr const char* objecttype = "elementary_step";

  // Inherit constructors
  using Object::Object;

  /**
   * @brief Creates a new reaction in the remote database.
   *
   * @param lhs The list of structures on the left hand side of the reaction.
   * @param rhs The list of structures on the right hand side of the reaction.
   * @param collection The collection to write the reaction into
   *
   * @throws MissingCollectionException Thrown if no collection is linked.
   * @returns A new instance
   */
  static ElementaryStep create(const std::vector<ID>& lhs, const std::vector<ID>& rhs, const CollectionPtr& collection);

  /**
   * @brief Creates a new reaction in the remote database.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param lhs The list of structures on the left hand side of the reaction.
   * @param rhs The list of structures on the right hand side of the reaction.
   * @return ID The ID of the newly inserted reaction.
   */
  ID create(const std::vector<ID>& lhs, const std::vector<ID>& rhs);
  /**
   * @brief Sets the type of the elementary step.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param type The new step type.
   */
  void setType(const ElementaryStepType& type) const;
  /**
   * @brief Gets the type of the elementary step.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return ElementaryStepType The type of the elementary step.
   */
  ElementaryStepType getType() const;
  /**
   * @brief Get linked compound-id
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the structure does not have a linked reaction ID.
   * @return ID The ID of the linked reaction.
   */
  ID getReaction() const;
  //! Fetch the linked reaction instance
  Reaction getReaction(const Manager& manager, const std::string& collection = Layout::DefaultCollection::reaction) const;
  /**
   * @brief Links the elementary step to a reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The new ID to link.
   */
  void setReaction(const ID& id) const;
  /**
   * @brief Whether the elementary step is linked to a reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  bool hasReaction() const;
  /**
   * @brief Removes the current link to a reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearReaction() const;
  /**
   * @brief Get linked transition state structure id.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the structure does not have a linked transition state ID.
   * @return ID The ID of the linked structure.
   */
  ID getTransitionState() const;
  //! Fetch the linked transition state structure
  Structure getTransitionState(const Manager& manager, const std::string& collection = Layout::DefaultCollection::structure) const;
  /**
   * @brief Links link the elementary step to a distinct transition state structure.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The new ID to link.
   */
  void setTransitionState(const ID& id) const;
  /**
   * @brief Checks if the elementary step contains a distinct transition state.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true  If the elementary step is linked to a transition state structure.
   * @return false If the elementary step is not linked to a transition state structure.
   */
  bool hasTransitionState() const;
  /**
   * @brief Removes the current transition state.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearTransitionState() const;
  /**
   * @brief Checks if a structure is part of the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id    The ID to be checked for.
   * @return SIDE The side of the reaction on which the ID was found.
   */
  SIDE hasReactant(const ID& id) const;
  /**
   * @brief Add a single reactant (Structure) to one or both sides of the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id   The structure id.
   * @param side The side(s) on which to work.
   */
  void addReactant(const ID& id, SIDE side) const;
  /**
   * @brief Remove a single reactant (Structure) from one or both sides of the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id   The structure id.
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
   * @brief Set the Reactants object
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param ids  The new structure ids.
   * @param side The side of the reaction on which to replace the structures-ids.
   */
  void setReactants(const std::vector<ID>& ids, SIDE side) const;
  /**
   * @brief Get the Reactants object
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param side The sides of which the ids should be returned.
   * @return std::tuple<std::vector<ID>, std::vector<ID>> The stored reactants.
   *         If the only one onf the sides was requested, the other part of the
   *         tuple will an empty vector.
   */
  std::tuple<std::vector<ID>, std::vector<ID>> getReactants(SIDE side) const;
  /**
   * @brief Removes the reactants of one or both sides of the reaction.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param side The side(s) of the reaction the clear the reactant of.
   */
  void clearReactants(SIDE side) const;
  /**
   * @brief Checks if data for a spline interpolation of the step is stored.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true If the data exists.
   * @return false If the data does not exist.
   */
  bool hasSpline() const;
  /**
   * @brief Get the Spline object.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return Utils::TrajectorySpline The (compressed) spline representing a fit
                                     to the trajectory.
   */
  Utils::BSplines::TrajectorySpline getSpline() const;
  /**
   * @brief Set the Spline object.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param spline The interpolation spline to attach to the elementary step.
   */
  void setSpline(const Utils::BSplines::TrajectorySpline& spline) const;
  /**
   * @brief Removes any currently stored spline.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearSpline() const;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_ELEMENTARYSTEP_H_ */
