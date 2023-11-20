/**
 * @file Calculation.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_CALCULATION_H_
#define DATABASE_CALCULATION_H_

/* Internal Include */
#include "Database/Layout.h"
#include "Database/Objects/Object.h"
/* External Includes */
#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

namespace Scine {
namespace Utils {
namespace UniversalSettings {
class ValueCollection;
class GenericValue;
} // namespace UniversalSettings
} // namespace Utils

namespace Database {

class Model;
class Manager;
class Structure;

/**
 * @class Calculation Calculation.h
 * @brief A blueprint for a database storable calculation.
 *
 * A calculation has the following fields:
 *  - 'structures':
 *    A list of IDs corresponding to Structure documents in the 'structure' collection of the same database.
 *  - \b 'results'
 *    The resulting data from the finished calculation, given as IDs of documents parsed into
 *    the database.
 *    Also see the Calculation::Results class bundling the data.
 *  - \b 'job'
 *    Information about the execution of the job, including the specific task and its hardware
 *    requirements. Also see the Calculation::Job class bundling the data.
 *  - \b 'model'
 *    The simulation model used in the calculation, e.g. PBE0/def2-tzvp COSMO(THF).
 *    For a comprehensive view on how the model is encoded see the Model class (Model.h).
 *  - \b 'settings'
 *    Additional settings beyond the Model and the Calculation::Job, technical details that
 *    are specific to the chosen Calculation::Job::order and Model. Examples: SCF convergence
 *    settings, or convergence criteria for a geometry optimization.
 *  - \b 'raw_output'
 *    A string containing the raw output of calculations that were parsed.
 *    Note that linked or compilation based interfaces for calculations may not produce this field.
 *  - \b 'status'
 *    The status of the calculation, see Calculation::STATUS enum for the values.
 *  - \b 'comment'
 *    A free form string, for additional human relevant information.
 *  - \b 'executor'
 *    A free form string, set to identify the runner/executor that handled the calculation.
 *  - \b 'restart_information'
 *    A dictionary/map/document of strings to IDs that may ease restarting this calculation
 *    with different settings to correct previous failures
 *  - \b 'auxiliaries'
 *    A dictionary/map/document of named IDs that may be useful when interpreting the results
 *    of the calculation. E.g. the calculation start by calculating a complex consisting of 2
 *    molecules, then the auxiliaries might point to the separate structures in the database.
 */
class Calculation : public Object {
 public:
  /// @brief The name of this derived database object.
  static constexpr const char* objecttype = "calculation";

  // Inherit constructors
  using Object::Object;

  /*=======*
   *  Job
   *=======*/

  /**
   * @class Job
   * @brief A small structure to collect the information about the specific job to
   *        be done in the calculation.
   */
  class Job {
   public:
    Job() = delete;
    /**
     * @brief Construct a new Job object.
     * @param o The order.
     */
    Job(std::string o) : order(std::move(o)){};
    /// @brief The actual order to be carried out, e.g. single_point
    std::string order;
    /// @brief Minimum required memory in GB.
    double memory = 1.0;
    /// @brief Minimum required number of cores.
    int cores = 1;
    /// @brief Minimum disk space in GB.
    double disk = 1.0;
    bool operator==(const Job& rhs) const {
      return this->order == rhs.order && this->cores == rhs.cores && std::fabs(this->memory - rhs.memory) < 1e-12 &&
             std::fabs(this->disk - rhs.disk) < 1e-12;
    }
    bool operator!=(const Job& rhs) const {
      return !(*this == rhs);
    }
  };
  /**
   * @brief Get the Job object.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return Job The current job details.
   */
  Job getJob() const;
  /**
   * @brief Set the Job object
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param job The new job details
   */
  void setJob(const Job& job) const;

  /*==================*
   *  Creation in DB
   *==================*/

  /**
   * @brief Creates a minimal version of a Calculation in the linked database.
   *
   * Generates a calculation with the STATUS::CONSTRUCTION in the database.
   * When finished with the setup of a the calculation, set the status to STATUS::NEW.
   *
   * @param model      The simulation model to be used.
   * @param job        The job specifications.
   * @param structures The structures used in this calculation given as IDs.
   * @param collection Collection to write this data into
   *
   * @throws MissingCollectionException if the collection pointer is empty
   *
   * @returns The new calculation instance.
   */
  static Calculation create(const Model& model, const Job& job, const std::vector<ID>& structures,
                            const CollectionPtr& collection);

  /**
   * @brief Creates a minimal version of a Calculation in the linked database.
   *
   * Generates a calculation with the STATUS::CONSTRUCTION in the database.
   * When finished with the setup of a the calculation, set the status to STATUS::NEW.
   *
   * Repeated calls to this function will generate new documents, the ID stored in
   * this instance of the class will always be the one of the last generated one.
   *
   * @throws MissingLinkedCollectionException if no collection is linked.
   *
   * @param model      The simulation model to be used.
   * @param job        The job specifications.
   * @param structures The structures used in this calculation given as IDs.
   * @return ID        The id of the inserted document.
   */
  ID create(const Model& model, const Job& job, const std::vector<ID>& structures);

  /*============*
   *  Priority
   *============*/

  /**
   * @brief Get the priority of this calculation.
   *
   * The priority is set on a scale of integers from 1 to 10, with 1 marking
   * the most important calculations.
   *
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return unsigned int The priority of this calculation.
   */
  unsigned getPriority() const;
  /**
   * @brief Set the priority.
   *
   * The priority is set on a scale of integers from 1 to 10, with 1 marking
   * the most important calculations.
   *
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws std::invalid_argument Thrown if the priority is not within [1:10].
   * @param priority The new priority.
   */
  void setPriority(unsigned priority) const;

  /*==========*
   *  Status
   *==========*/

  /**
   * @brief The set of all status states possible for a calculation.
   */
  using STATUS = CalculationStatus;
  /**
   * @brief Get the status.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return STATUS The current status.
   */
  STATUS getStatus() const;
  /**
   * @brief Set the status.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param status The new status.
   */
  void setStatus(STATUS status) const;

  /*=========*
   *  Model
   *=========*/

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

  /*==============*
   *  Structures
   *==============*/

  /**
   * @brief Adds the id of a single structure.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The id of a structure.
   */
  void addStructure(const ID& id) const;
  /**
   * @brief Removes a single structure-id form the list.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The id of a structure.
   */
  void removeStructure(const ID& id) const;
  /**
   * @brief Checks if a particular structure-id is present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param id The id of a structure.
   * @return true  If a structure with the given id is present in the list.
   * @return false If no structure with the given id is present in the list.
   */
  bool hasStructure(const ID& id) const;
  /**
   * @brief Get all structures-ids in a vector.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the object not be found.
   * @return std::vector<ID> The vector of IDs.
   */
  std::vector<ID> getStructures() const;
  /**
   * @brief Get all structures-ids in a vector.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if this instance does not have an ID.
   * @throws MissingIdOrField Thrown if the object wasn't found.
   * @returns A vector of structures.
   */
  std::vector<Structure> getStructures(const Manager& manager,
                                       const std::string& collection = Layout::DefaultCollection::structure) const;
  /**
   * @brief Removes all stored structures.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearStructures() const;
  /**
   * @brief Set (replace) all structure-ids with a given list.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param structures A vector of structure-ids.
   */
  void setStructures(const std::vector<ID>& structures) const;

  /*============*
   *  Settings
   *============*/

  /**
   * @brief Set a setting.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key   The key under which the setting is to be stored.
   * @param value The value of the setting.
   */
  void setSetting(const std::string& key, const Utils::UniversalSettings::GenericValue& value) const;
  /**
   * @brief Get a setting by key.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the object or key can not be found.
   * @param key The key of a setting.
   * @return std::string The value of the setting.
   */
  Utils::UniversalSettings::GenericValue getSetting(const std::string& key) const;
  /**
   * @brief Check if a setting with a particular key exists.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of a setting.
   * @return true  If a setting with the given key exists.
   * @return false If no setting with the given key exists.
   */
  bool hasSetting(const std::string& key) const;
  /**
   * @brief Remove a single setting by key.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of a setting.
   */
  void removeSetting(const std::string& key) const;
  /**
   * @brief Clear all existing settings.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearSettings() const;
  /**
   * @brief Get all settings.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the object not be found.
   * @return The current settings.
   */
  Utils::UniversalSettings::ValueCollection getSettings() const;
  /**
   * @brief Set (replace) all settings.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param settings The new settings.
   */
  void setSettings(const Utils::UniversalSettings::ValueCollection& settings) const;

  /*===========*
   *  Results
   *===========*/

  /**
   * @class Results
   * @brief A small structure to collect the results generated from calculations.
   */
  struct Results {
    /// @brief The generated properties (Property).
    std::vector<ID> properties;
    /// @brief The generated structures (Structure).
    std::vector<ID> structures;
    /// @brief The generated reaction paths (ElementaryStep).
    std::vector<ID> elementarySteps;

    Results operator+(const Results& other) const {
      auto merge = Results(*this);
      merge += other;
      return merge;
    };

    Results operator+=(const Results& other) {
      // combine vectors
      this->properties.insert(this->properties.end(), other.properties.begin(), other.properties.end());
      this->structures.insert(this->structures.end(), other.structures.begin(), other.structures.end());
      this->elementarySteps.insert(this->elementarySteps.end(), other.elementarySteps.begin(), other.elementarySteps.end());
      // lambda function to remove duplicates from vector
      auto removeDuplicates = [](std::vector<Database::ID>& v) {
        auto end = v.end();
        for (auto it = v.begin(); it != end; ++it) {
          end = std::remove(it + 1, end, *it);
        }
        v.erase(end, v.end());
      };
      removeDuplicates(this->properties);
      removeDuplicates(this->structures);
      removeDuplicates(this->elementarySteps);
      return *this;
    }
  };
  /**
   * @brief Set the results.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param results The new results object.
   */
  void setResults(Results& results) const;
  /**
   * @brief Get the results.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the object not be found.
   * @return Results The current results.
   */
  Results getResults() const;
  /**
   * @brief Clears all results that are currently present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearResults() const;

  /*===============*
   *  Auxiliaries
   *===============*/

  /**
   * @brief Check if an auxiliary with the given key is present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the auxiliary.
   * @return true  If an auxiliary with the given key is present.
   * @return false If no auxiliary with the given key is present.
   */
  bool hasAuxiliary(std::string key) const;
  /**
   * @brief Set an auxiliary.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the auxiliary.
   * @param id The new ID.
   */
  void setAuxiliary(std::string key, const ID& id) const;
  /**
   * @brief Get an auxiliary
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the object or key can not be found.
   * @param key The key of the auxiliary.
   * @return ID The current ID.
   */
  ID getAuxiliary(std::string key) const;
  /**
   * @brief Remove a single auxiliary if present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the auxiliary.
   */
  void removeAuxiliary(std::string key) const;
  /**
   * @brief Set (replace) all auxiliaries at once.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param auxiliaries The new auxiliaries
   */
  void setAuxiliaries(std::map<std::string, ID> auxiliaries) const;
  /**
   * @brief Get all auxiliaries as a map.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the object not be found.
   * @return std::map<std::string, ID> All auxiliaries.
   */
  std::map<std::string, ID> getAuxiliaries() const;
  /**
   * @brief Clears all currently existing auxiliaries.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearAuxiliaries() const;

  /*======================*
   *  Restart Information
   *======================*/

  /**
   * @brief Check if a restart information with the given key is present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the restart information.
   * @return true  If a restart information with the given key is present.
   * @return false If no restart information with the given key is present.
   */
  bool hasRestartInformation(const std::string& key) const;
  /**
   * @brief Set a restart information.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the restart information.
   * @param id The new ID.
   */
  void setRestartInformation(const std::string& key, const ID& id) const;
  /**
   * @brief Get a restart information
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the object or key cannot be found.
   * @param key The key of the restart information.
   * @return ID The current ID.
   */
  ID getRestartInformation(const std::string& key) const;
  /**
   * @brief Remove a single restart information if present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param key The key of the restart information.
   */
  void removeRestartInformation(const std::string& key) const;
  /**
   * @brief Set (replace) all restart information at once.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param restart information The new restart information.
   */
  void setRestartInformation(const std::map<std::string, ID>& restartInformation) const;
  /**
   * @brief Get restart information as a map.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @throws MissingIdOrField Thrown if the object not be found.
   * @return std::map<std::string, ID> Total restart information.
   */
  std::map<std::string, ID> getRestartInformation() const;
  /**
   * @brief Clears all currently existing restart information.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearRestartInformation() const;

  /*=============*
   *  Raw Ouput
   *=============*/

  /**
   * @brief Get the raw output.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::string The raw output.
   */
  std::string getRawOutput() const;
  /**
   * @brief Set the raw output.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param output The raw output.
   */
  void setRawOutput(const std::string& output) const;
  /**
   * @brief Checks if raw output is present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true  If a raw output is present.
   * @return false If no raw output is present.
   */
  bool hasRawOutput() const;
  /**
   * @brief Removes any raw output currently present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearRawOutput() const;

  /*===========*
   *  Comment
   *===========*/

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

  /*===========*
   *  Executor
   *===========*/

  /**
   * @brief Get the executor.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::string The executor.
   */
  std::string getExecutor() const;
  /**
   * @brief Set the executor.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param executor The executor.
   */
  void setExecutor(const std::string& executor) const;
  /**
   * @brief Checks if a executor is present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true  If a executor is present.
   * @return false If no executor is present.
   */
  bool hasExecutor() const;
  /**
   * @brief Removes any executor currently present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearExecutor() const;

  /*===========*
   *  Runtime
   *===========*/

  /**
   * @brief Get the runtime in seconds.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return std::string The runtime in seconds.
   */
  double getRuntime() const;
  /**
   * @brief Set the runtime in seconds.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @param runtime The runtime in seconds.
   */
  void setRuntime(double runtime) const;
  /**
   * @brief Checks if a runtime is present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   * @return true  If a runtime is present.
   * @return false If no runtime is present.
   */
  bool hasRuntime() const;
  /**
   * @brief Removes any runtime currently present.
   * @throws MissingLinkedCollectionException Thrown if no collection is linked.
   * @throws MissingIDException Thrown if the object does not have an ID.
   */
  void clearRuntime() const;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_CALCULATION_H_ */
