/**
 * @file CalculationPython.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include "Helpers.h"
#include <Database/Collection.h>
#include <Database/Exceptions.h>
#include <Database/Manager.h>
#include <Database/Objects/Calculation.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/Property.h>
#include <Database/Objects/Structure.h>
#include <Utils/Pybind.h>
#include <Utils/UniversalSettings/GenericValueVariant.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace Scine::Database;

void init_calculation(pybind11::module& m) {
  pybind11::class_<Calculation::Results> results(m, "Results",
                                                 R"delim(
      A data class representing database object ids resulting from a calculation

      ..note: This class owns its data and is not conceptually a database
              pointer similar to the database objects.

      Three types of database objects are modeled to be calculation results:
      properties (``Property``), structures (``Structure``) and elementary steps
      (``ElementaryStep``). This type tracks the IDs of these objects.

      :example:
      >>> results = Results()
      >>> results.property_ids
      []
      >>> results.property_ids = [ID()]  # Note: not a DB operation
      >>> len(results.property_ids)
      1
    )delim");
  results.def(pybind11::init<>());
  results.def_readwrite("property_ids", &Calculation::Results::properties, "Generated property object ids");
  results.def_readwrite("structure_ids", &Calculation::Results::structures, "Generated structure object ids");
  results.def_readwrite("elementary_step_ids", &Calculation::Results::elementarySteps,
                        "Generated reaction path elementary step object ids");

  results.def("clear", [](Calculation::Results& results) {
    results.properties.clear();
    results.structures.clear();
    results.elementarySteps.clear();
  });
  results.def("add_property", [](Calculation::Results& results, const ID& id) { results.properties.push_back(id); });
  results.def("remove_property", [](Calculation::Results& results, const ID& id) {
    results.properties.erase(std::remove(results.properties.begin(), results.properties.end(), id), results.properties.end());
  });
  results.def("clear_properties", [](Calculation::Results& results) { results.properties.clear(); });
  results.def("set_properties",
              [](Calculation::Results& results, const std::vector<ID>& ids) { results.properties = ids; });
  results.def("get_properties", [](Calculation::Results& results) { return results.properties; });
  results.def("get_property", [](Calculation::Results& results, const int& idx) { return results.properties[idx]; });
  results.def("get_property", [](Calculation::Results& results, const std::string& key, const Object::CollectionPtr& coll) {
    for (const auto& id : results.properties) {
      auto prop = Property(id);
      prop.link(coll);
      if (prop.getPropertyName() == key) {
        return id;
      }
    }
    throw Exceptions::FieldException();
  });

  results.def("add_elementary_step",
              [](Calculation::Results& results, const ID& id) { results.elementarySteps.push_back(id); });
  results.def("remove_elementary_step", [](Calculation::Results& results, const ID& id) {
    results.elementarySteps.erase(std::remove(results.elementarySteps.begin(), results.elementarySteps.end(), id),
                                  results.elementarySteps.end());
  });
  results.def("clear_elementary_steps", [](Calculation::Results& results) { results.elementarySteps.clear(); });
  results.def("set_elementary_steps",
              [](Calculation::Results& results, const std::vector<ID>& ids) { results.elementarySteps = ids; });
  results.def("get_elementary_steps", [](Calculation::Results& results) { return results.elementarySteps; });
  results.def("get_elementary_step",
              [](Calculation::Results& results, const int& idx) { return results.elementarySteps[idx]; });

  results.def("add_structure", [](Calculation::Results& results, const ID& id) { results.structures.push_back(id); });
  results.def("remove_structure", [](Calculation::Results& results, const ID& id) {
    results.structures.erase(std::remove(results.structures.begin(), results.structures.end(), id), results.structures.end());
  });
  results.def("clear_structures", [](Calculation::Results& results) { results.structures.clear(); });
  results.def("set_structures",
              [](Calculation::Results& results, const std::vector<ID>& ids) { results.structures = ids; });
  results.def("get_structures", [](Calculation::Results& results) { return results.structures; });
  results.def("get_structure", [](Calculation::Results& results, const int& idx) { return results.structures[idx]; });

  // Addition operators
  results.def(pybind11::self + pybind11::self);
  results.def(pybind11::self += pybind11::self);

  pybind11::class_<Calculation::Job> job(m, "Job",
                                         R"delim(
      A data class representing execution details of the calculation.

      ..note: This class owns its data and is not conceptually a database
              pointer similar to the database objects.

      A Job comprises only an identifier and minimum requirements on available
      memory, computer cores and disk space.

      :example:
      >>> job = Job("single_point")
      >>> job.cores = 4  # Four computer cores
      >>> job.disk = 2.0  # 2 GB of disk space
      >>> job.memory
      1.0
    )delim");
  job.def("__copy__", [](const Calculation::Job& self) -> Calculation::Job { return Calculation::Job(self); });
  job.def("__deepcopy__", [](const Calculation::Job& self, pybind11::dict /* memo */) -> Calculation::Job {
    return Calculation::Job(self);
  });
  job.def(pybind11::init<std::string>(), pybind11::arg("order"));
  job.def_readwrite("order", &Calculation::Job::order,
                    "Summary string of the type of job to be carried out, e.g. 'single_point'");
  job.def_readwrite("memory", &Calculation::Job::memory, "Minimum required memory in GB");
  job.def_readwrite("cores", &Calculation::Job::cores, "Minimum required number of cores");
  job.def_readwrite("disk", &Calculation::Job::disk, "Minimum required disk space in GB");
  job.def(pybind11::pickle(
      [](const Calculation::Job& j) { // __getstate__
        /* Return a tuple that fully encodes the state of the object */
        return pybind11::make_tuple(j.order, j.memory, j.cores, j.disk);
      },
      [](pybind11::tuple t) { // __setstate__
        if (t.size() != 4)
          throw std::runtime_error("Invalid state for Job!");

        /* Create a new C++ instance */
        Calculation::Job j(t[0].cast<std::string>());

        j.memory = t[1].cast<double>();
        j.cores = t[2].cast<int>();
        j.disk = t[3].cast<double>();

        return j;
      }));
  job.def(pybind11::self == pybind11::self);
  job.def(pybind11::self != pybind11::self);
  job.def("__copy__", [](const Calculation::Job& self) -> Calculation::Job { return Calculation::Job(self); });
  job.def("__deepcopy__", [](const Calculation::Job& self, pybind11::dict /* memo */) -> Calculation::Job {
    return Calculation::Job(self);
  });

  pybind11::enum_<Calculation::STATUS> status(m, "Status");
  status.value("CONSTRUCTION", Calculation::STATUS::CONSTRUCTION,
               "Calculation database representation is work in progress, is not ready to be run");
  status.value("NEW", Calculation::STATUS::NEW, "Calculation is scheduled to run");
  status.value("PENDING", Calculation::STATUS::PENDING, "Calculation is running, awaiting results");
  status.value("COMPLETE", Calculation::STATUS::COMPLETE, "Calculation is complete");
  status.value("ANALYZED", Calculation::STATUS::ANALYZED, "The calculation has been post-processed");
  status.value("HOLD", Calculation::STATUS::HOLD, "The calculation is being held ready for scheduling");
  status.value("FAILED", Calculation::STATUS::FAILED, "The calculation failed to complete successfully");

  pybind11::class_<Calculation, Object> calculation(m, "Calculation",
                                                    R"delim(
      Class referencing a calculation database object

      A Calculation is conceptualized as the data necessary to schedule, run
      and store the results of a time-intensive calculation.

      Calculations have the following relationships to other database objects:
      - ``Structure``: Calculations are performed on input structures and can
        produce output structures.
      - ``Property``: Calculations can generate new properties on input or output
        structures
      - ``ElementaryStep``: Calculations may generate elementary step objects
        linking structures by transition states

      Calculations' data representation consists of many parts. Conceptually,
      it can be separated into scheduling details, inputs and outputs. The
      scheduling details comprise the ``Job`` data, the ``priority``,
      ``status`` and ``executor``.

      :example:
      >>> collection = manager.get_collection("calculations")
      >>> model = Model("dft", "pbe", "def2-svp")
      >>> job = Job("single_point")
      >>> calculation = Calculation.make(model, job, [ID(), ID()], collection)
      >>> calculation.status == Status.CONSTRUCTION
      True
      >>> calculation.priority
      10
      >>> calculation.model.method_family = "pffft"  # WARNING: This does nothing!
      >>> calculation.model.method_family
      'dft'
      >>> model.method_family = "pffft"  # Manipulate a local dataclass instance
      >>> calculation.model = model  # Overwrite the database representation
      >>> calculation.model.method_family
      'pffft'

      The inputs of the calculation are composed of the input structures and
      the ``settings``. Settings are a specially typed collection of various
      data structures, in what is essentially a nested string-any dictionary.
      The settings allow data specific to the job to be abstracted away in
      a regularized form.

      :example:
      >>> collection = manager.get_collection("calculations")
      >>> job = Job("remy")
      >>> model = Model("cooking", "heat", "oven")
      >>> calculation = Calculation.make(model, job, [ID(), ID()], collection)
      >>> import scine_utilities as utils
      >>> settings = utils.ValueCollection()
      >>> settings["temperature"] = 180.0
      >>> settings["temperature_unit"] = "celsius"
      >>> ingredients = {"tomatoes": 20, "kohlrabi": 2, "chillies": 1, "olive_oil": True}
      >>> settings["ingredients"] = utils.ValueCollection(ingredients)  # Nesting!
      >>> calculation.set_settings(settings)

      The outputs of a calculation are stored in ``raw_output``,
      ``auxiliaries`` and ``results``. The raw output generally lists captured
      standard output and error output of the job. Auxiliaries are intended as
      relatively hints for interpreting the results (e.g. labels for a new
      structure or relationships between them). ``Results`` group
      database objects generated during calculation execution.
    )delim");
  calculation.def(pybind11::init<>());
  calculation.def(pybind11::init<const ID&>());
  calculation.def(pybind11::init<const ID&, const Object::CollectionPtr&>());

  calculation.def_static(
      "make",
      pybind11::overload_cast<const Model&, const Calculation::Job&, const std::vector<ID>&, const Object::CollectionPtr&>(
          &Calculation::create),
      pybind11::arg("model"), pybind11::arg("job"), pybind11::arg("structures"), pybind11::arg("collection"));
  calculation.def("create",
                  pybind11::overload_cast<const Model&, const Calculation::Job&, const std::vector<ID>&>(&Calculation::create),
                  pybind11::arg("model"), pybind11::arg("job"), pybind11::arg("structures"));

  calculation.def("get_job", &Calculation::getJob);
  calculation.def("set_job", &Calculation::setJob, pybind11::arg("job"));
  calculation.def_property("job", &Calculation::getJob, &Calculation::setJob);

  calculation.def("get_priority", &Calculation::getPriority);
  calculation.def("set_priority", &Calculation::setPriority, pybind11::arg("priority"));
  calculation.def_property("priority", &Calculation::getPriority, &Calculation::setPriority);

  calculation.def("get_status", &Calculation::getStatus);
  calculation.def("set_status", &Calculation::setStatus, pybind11::arg("status"));
  calculation.def_property("status", &Calculation::getStatus, &Calculation::setStatus, "Stage of a calculation's existence");

  calculation.def("get_model", &Calculation::getModel);
  calculation.def("set_model", &Calculation::setModel, pybind11::arg("model"));
  calculation.def_property("model", &Calculation::getModel, &Calculation::setModel,
                           "The methodological details to apply in the calculation");

  calculation.def("add_structure", &Calculation::addStructure, pybind11::arg("id"));
  calculation.def("remove_structure", &Calculation::removeStructure, pybind11::arg("id"));
  calculation.def("has_structure", &Calculation::hasStructure, pybind11::arg("id"));
  calculation.def("get_structures", pybind11::overload_cast<>(&Calculation::getStructures, pybind11::const_));
  calculation.def("get_structures",
                  pybind11::overload_cast<const Manager&, const std::string&>(&Calculation::getStructures, pybind11::const_),
                  pybind11::arg("manager"), pybind11::arg("collection"));
  calculation.def("clear_structures", &Calculation::clearStructures);
  calculation.def("set_structures", &Calculation::setStructures, pybind11::arg("structures"));

  calculation.def(
      "set_setting",
      [](Calculation& calculation, const std::string& key,
         const Scine::Utils::UniversalSettings::GenericValueMeta::Variant& value) {
        calculation.setSetting(key, Scine::Utils::UniversalSettings::GenericValueMeta::convert(value));
      },
      pybind11::arg("key"), pybind11::arg("value"));
  calculation.def(
      "get_setting",
      [](const Calculation& calculation, const std::string& key) {
        return Scine::Utils::UniversalSettings::GenericValueMeta::convert(calculation.getSetting(key));
      },
      pybind11::arg("key"));
  calculation.def("has_setting", &Calculation::hasSetting, pybind11::arg("key"));
  calculation.def("remove_setting", &Calculation::removeSetting, pybind11::arg("key"));
  calculation.def("clear_settings", &Calculation::clearSettings);
  calculation.def("get_settings", &Calculation::getSettings);
  calculation.def("set_settings", &Calculation::setSettings, pybind11::arg("settings"));

  calculation.def("set_results", &Calculation::setResults, pybind11::arg("results"));
  calculation.def("get_results", &Calculation::getResults);
  calculation.def("clear_results", &Calculation::clearResults);

  calculation.def("has_auxiliary", &Calculation::hasAuxiliary, pybind11::arg("key"));
  calculation.def("set_auxiliary", &Calculation::setAuxiliary, pybind11::arg("key"), pybind11::arg("id"));
  calculation.def("get_auxiliary", &Calculation::getAuxiliary, pybind11::arg("key"));
  calculation.def("remove_auxiliary", &Calculation::removeAuxiliary, pybind11::arg("key"));
  calculation.def("set_auxiliaries", &Calculation::setAuxiliaries, pybind11::arg("auxiliaries"));
  calculation.def("get_auxiliaries", &Calculation::getAuxiliaries);
  calculation.def("clear_auxiliaries", &Calculation::clearAuxiliaries);

  calculation.def("get_raw_output", &Calculation::getRawOutput);
  calculation.def("set_raw_output", &Calculation::setRawOutput, pybind11::arg("raw_output"));
  calculation.def("has_raw_output", &Calculation::hasRawOutput);
  calculation.def("clear_raw_output", &Calculation::clearRawOutput);
  def_optional_property<Calculation>(calculation, "raw_output", std::mem_fn(&Calculation::hasRawOutput),
                                     std::mem_fn(&Calculation::getRawOutput), std::mem_fn(&Calculation::setRawOutput),
                                     std::mem_fn(&Calculation::clearRawOutput), "Raw output of the calculation");

  calculation.def("get_comment", &Calculation::getComment);
  calculation.def("set_comment", &Calculation::setComment, pybind11::arg("comment"));
  calculation.def("has_comment", &Calculation::hasComment);
  calculation.def("clear_comment", &Calculation::clearComment);
  def_optional_property<Calculation>(calculation, "comment", std::mem_fn(&Calculation::hasComment),
                                     std::mem_fn(&Calculation::getComment), std::mem_fn(&Calculation::setComment),
                                     std::mem_fn(&Calculation::clearComment), "Comment for the calculation");

  calculation.def("get_executor", &Calculation::getExecutor);
  calculation.def("set_executor", &Calculation::setExecutor, pybind11::arg("executor"));
  calculation.def("has_executor", &Calculation::hasExecutor);
  calculation.def("clear_executor", &Calculation::clearExecutor);
  def_optional_property<Calculation>(calculation, "executor", std::mem_fn(&Calculation::hasExecutor),
                                     std::mem_fn(&Calculation::getExecutor), std::mem_fn(&Calculation::setExecutor),
                                     std::mem_fn(&Calculation::clearExecutor),
                                     "Identifies the program/runner that executes the calculation");

  calculation.def("get_runtime", &Calculation::getRuntime);
  calculation.def("set_runtime", &Calculation::setRuntime, pybind11::arg("runtime"));
  calculation.def("has_runtime", &Calculation::hasRuntime);
  calculation.def("clear_runtime", &Calculation::clearRuntime);
  def_optional_property<Calculation>(calculation, "runtime", std::mem_fn(&Calculation::hasRuntime),
                                     std::mem_fn(&Calculation::getRuntime), std::mem_fn(&Calculation::setRuntime),
                                     std::mem_fn(&Calculation::clearRuntime), "Runtime of the calculation in seconds");
}
