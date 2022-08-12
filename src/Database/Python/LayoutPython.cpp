/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include <Database/Layout.h>
#include <Database/Objects/Structure.h>
#include <Utils/Pybind.h>

using namespace Scine::Database;

void init_layout(pybind11::module& m) {
  pybind11::module layout = m.def_submodule("layout");
  layout.doc() = "Meta-information on the mongodb layout of the database";
  layout.def(
      "default_collection",
      [](pybind11::object type) -> std::string {
        static std::unordered_map<std::string, std::string> map{
            {"Calculation", Layout::DefaultCollection::calculation},
            {"Compound", Layout::DefaultCollection::compound},
            {"Property", Layout::DefaultCollection::property},
            {"BoolProperty", Layout::DefaultCollection::property},
            {"NumberProperty", Layout::DefaultCollection::property},
            {"StringProperty", Layout::DefaultCollection::property},
            {"VectorProperty", Layout::DefaultCollection::property},
            {"DenseMatrixProperty", Layout::DefaultCollection::property},
            {"SparseMatrixProperty", Layout::DefaultCollection::property},
            {"Structure", Layout::DefaultCollection::structure},
            {"Reaction", Layout::DefaultCollection::reaction},
            {"ElementaryStep", Layout::DefaultCollection::elementaryStep},
        };
        std::string name = pybind11::str(type.attr("__name__"));
        auto findIter = map.find(name);
        if (findIter == map.end()) {
          throw std::runtime_error("The type '" + name + "' does not have a default collection");
        }
        return findIter->second;
      },
      pybind11::arg("db_type"),
      R"delim(
      Returns the default database collection name for a database object

      >>> import scine_database as db
      >>> default_collection(db.Structure)
      "structures"
      >>> default_collection(db.NumberProperty)
      "properties"
      >>> default_collection(db.Manager)
      Traceback (most recent call last):
        File "<stdin>", line 1, in <module>
      RuntimeError: The type 'Manager' does not have a default collection
    )delim");
  layout.def(
      "calculation_status",
      [](const CalculationStatus status) -> std::string { return Layout::EnumMaps::status2str.at(status); },
      pybind11::arg("status"),
      R"delim(
      Returns the database layout of a calculation status

      >>> import scine_database as db
      >>> calculation_status(db.Status.NEW)
      "new"
    )delim");
  layout.def(
      "structure_label", [](const StructureLabel label) -> std::string { return Layout::EnumMaps::label2str.at(label); },
      pybind11::arg("label"),
      R"delim(
      Returns the database layout of a structure label

      >>> import scine_database as db
      >>> structure_label(db.Label.MINIMUM_OPTIMIZED)
      "minimum_optimized"
    )delim");
  layout.def(
      "elementary_step_type",
      [](const ElementaryStepType type) -> std::string { return Layout::EnumMaps::estype2str.at(type); },
      pybind11::arg("type"),
      R"delim(
      Returns the database layout of a elementary step type

      >>> import scine_database as db
      >>> elementary_step_type(db.ElementaryStepType.REGULAR)
      "regular"
    )delim");
}
