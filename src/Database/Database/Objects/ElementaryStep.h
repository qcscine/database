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
#include <boost/optional.hpp>
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
   * @brief Creates a new elementary step in the remote database.
   *
   * @param lhs The list of structures on the left hand side of the elementary step.
   * @param rhs The list of structures on the right hand side of the elementary step.
   * @param collection The collection to write the elementary step into
   *
   * @throws MissingCollectionException Thrown if no collection is linked.
   * @returns A new instance
   */
  static ElementaryStep create(const std::vector<ID>& lhs, const std::vector<ID>& rhs, const CollectionPtr& collection);

  /**
   * @brief Creates a new elementary step in the remote database.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param lhs The list of structures on the left hand side of the elementary step.
   * @param rhs The list of structures on the right hand side of the elementary step.
   * @return ID The ID of the newly inserted elementary step.
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
   * @brief Checks if a structure is part of the elementary step.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id    The ID to be checked for.
   * @return SIDE The side of the elementary step on which the ID was found.
   */
  SIDE hasReactant(const ID& id) const;
  /**
   * @brief Add a single reactant (Structure) to one or both sides of the elementary step.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id   The structure id.
   * @param side The side(s) on which to work.
   */
  void addReactant(const ID& id, SIDE side) const;
  /**
   * @brief Remove a single reactant (Structure) from one or both sides of the elementary step.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id   The structure id.
   * @param side The side(s) on which to work.
   */
  void removeReactant(const ID& id, SIDE side) const;
  /**
   * @brief Count the number of reactants on both sides of the elementary step.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::tuple<int, int> The number of reactants on the sides of the elementary step.
   */
  std::tuple<int, int> hasReactants() const;
  /**
   * @brief Set the Reactants object
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param ids  The new structure ids.
   * @param side The side of the elementary step on which to replace the structures-ids.
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
   * @brief Removes the reactants of one or both sides of the elementary step.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param side The side(s) of the elementary step to clear the reactant of.
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

  /**
   * @brief Types of atom index maps between the structures linked by an elementary step.
   */
  enum class IdxMapType { LHS_TS, LHS_RHS, TS_LHS, RHS_LHS, TS_RHS, RHS_TS };

  /**
   * @brief Adds atom index map(s) to the elementary step.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param lhsRhsMap The atom index map between the stacked lhs and stacked rhs structures.
   * @param lhsTsMap The atom index map between the stacked lhs and stacked ts structures (optional).
   */
  void addIdxMaps(const std::vector<int>& lhsRhsMap, const boost::optional<std::vector<int>>& lhsTsMap = boost::none) const;

  /**
   * @brief Removes the atom index maps.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   */
  void removeIdxMaps() const;

  /**
   * @brief Checks whether an index map with the given name exists for the
   * elementary step or can be generated from the existing ones.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param mapType The type of the atom index map of interest.
   * @return bool True, if a map with the given name exists or can be retrieved from existing ones.
   */
  bool hasIdxMap(const IdxMapType& mapType) const;

  /**
   * @brief Gets the atom index map of the given type.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIdOrField Thrown if there is no such index map attached to the elementary step
   * and it cannot be retrieved from the existing ones.
   * @param mapType The type of the atom index map.
   * @return std::vector<int> The atom index map.
   */
  std::vector<int> getIdxMap(const IdxMapType& mapType) const;
  /**
   * @class InvalidIdxMapException
   * @brief An exception to throw if an atom index map is invalid e.g. not bijective.
   */
  class InvalidIdxMapException : public std::exception {
   public:
    const char* what() const throw() {
      return "The requested atom index map does not constitute a valid map.";
    }
  };
  /**
   * @brief Check if the ElementarySteps holds the queried ID as a Structure in its path.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The ID of the Structure to be checked for.
   * @return true  If the Structure ID is part of the ElementarySteps path.
   * @return false If the Structure ID is not part of the ElementarySteps path.
   */
  bool hasStructureInPath(const ID& id) const;
  /**
   * @brief Checks for the amount of structures in the ElementarySteps discretized path.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return int The number of structures linked to the ElementarySteps discretized path.
   */
  int hasPath() const;
  /**
   * @brief Get the all stored structures in the ElementarySteps discretized path.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::vector<ID> The vector of all structure ids.
   */
  std::vector<ID> getPath() const;
  //! Fetch all stored structures
  std::vector<Structure> getPath(const Manager& manager,
                                 const std::string& collection = Layout::DefaultCollection::structure) const;
  /**
   * @brief Set/replace all structures in the ElementarySteps discretized path.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param ids The new structures, in order: from LHS to RHS.
   */
  void setPath(const std::vector<ID>& ids) const;
  /**
   * @brief Removes all structures in the ElementarySteps discretized path.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearPath() const;
  /**
   * @brief Calculate the reaction barrier from the data behind the spline. This does
   *        not rely on the spline interpolation!
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return The reaction barriers as a tuple (lhs, rhs). Returns zero if no
   *         spline is available.
   */
  std::tuple<double, double> getBarrierFromSpline() const;

 private:
  /**
   * @brief Checks whether an index map with the given name exists for the
   * elementary step or can be generated from the existing ones.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @param key The name of the atom index map of interest.
   * @return bool True, if a map with the given name exists for the elementary step.
   */
  bool hasIdxMapByKey(const std::string& key) const;

  /**
   * @brief Gets the atom index map with the given name.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIdOrField Thrown if there is no such index map attached to the elementary step.
   * @param key The name of the atom index map.
   * @return std::vector<int> The atom index map.
   */
  std::vector<int> getIdxMapByKey(const std::string& key) const;

  /**
   * @brief Constructs a vector where indices and values are swapped w.r.t. to the input vector.
   * @throws InvalidIdxMapException Thrown if the input vector's sorted values unequal its indices.
   * @param std::vector<int> The input vector
   * @return std::vector<int> A vector of the size of the input vector but with indices and values swapped.
   */
  std::vector<int> reverseIdxMap(const std::vector<int>& unswapped) const;

  /**
   * @brief Gets a chained atom index map arising from applying both specified
   * maps subsequently.
   * For example, if the first vector represents a mapping from lhs atom indices
   * to ts atom indices and the second ts atom indices to  rhs atom indices
   * their chain maps lhs atom indices to rhs atom indices.
   * @throws InvalidIdxMapException Thrown if the two maps are of unequal size or if at least one contains
   * any element with a value larger than the map size.
   * @param idxMap1 The name of the first input map.
   * @param idxMap2 The name of the second input map.
   * @return std::vector<int> The chained atom index map.
   */
  std::vector<int> chainIdxMaps(const ::std::vector<int>& idxMap1, const ::std::vector<int>& idxMap2) const;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_ELEMENTARYSTEP_H_ */
