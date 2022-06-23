/**
 * @file PythonModule.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include "Database/Collection.h"
#include "Database/Manager.h"
#include "Database/Objects/Compound.h"
#include "Database/Objects/Reaction.h"
#include "Database/Objects/Structure.h"
#include <Database/Version.h>
#include <Utils/Pybind.h>

using namespace Scine::Database;

struct PybindTypes {
  using PyCollection = pybind11::class_<Collection, std::shared_ptr<Collection>>;
  using PyManager = pybind11::class_<Manager>;
  using PyObject = pybind11::class_<Object>;
  using PyReaction = pybind11::class_<Reaction, Object>;
  using PyStructure = pybind11::class_<Structure, Object>;
  using PyCompound = pybind11::class_<Compound, Object>;

  PybindTypes(pybind11::module& m)
    : collection(m, "Collection"),
      manager(m, "Manager"),
      object(m, "Object"),
      reaction(m, "Reaction"),
      structure(m, "Structure"),
      compound(m, "Compound") {
  }

  PyCollection collection;
  PyManager manager;
  PyObject object;
  PyReaction reaction;
  PyStructure structure;
  PyCompound compound;
};

/* Basic Database Classes */
void init_manager(PybindTypes::PyManager&);
void init_collection(PybindTypes::PyCollection&);
/* Other definitions */
void init_reaction_side(pybind11::module&);
void init_misc(pybind11::module&);
/* Database Objects */
void init_model(pybind11::module&);
void init_id(pybind11::module&);
void init_object(PybindTypes::PyObject&);
void init_calculation(pybind11::module&);
void init_elementary_step(pybind11::module&);
void init_structure_label(pybind11::module&);
void init_structure(PybindTypes::PyStructure&);
void init_compound(PybindTypes::PyCompound&);
void init_properties(pybind11::module&);
void init_credentials(pybind11::module&);
void init_reaction(PybindTypes::PyReaction&);
void init_layout(pybind11::module&);

struct VersionNamespace {
  VersionNamespace() = delete;
};

PYBIND11_MODULE(scine_database, m) {
  m.doc() = "Python bindings for SCINE-Database";

  /* Try to preload scine_utilities for type annotations in the pybind
   * docstrings. They're not necessary though, this module works without, too.
   */
  try {
    pybind11::module::import("scine_utilities");
  }
  catch (pybind11::error_already_set& e) {
  }

  PybindTypes types(m);

  // Expose current version
  auto dbVersion = pybind11::class_<VersionNamespace>(m, "database_version");
  dbVersion.def_readonly_static("major", &Version::major);
  dbVersion.def_readonly_static("minor", &Version::minor);
  dbVersion.def_readonly_static("patch", &Version::patch);
  dbVersion.def("__repr__", []() {
    return std::to_string(Version::major) + "." + std::to_string(Version::minor) + "." + std::to_string(Version::patch);
  });

  // Other definitions
  init_reaction_side(m);
  init_misc(m);
  init_id(m);

  // Database Objects
  init_object(types.object);
  init_model(m);
  init_elementary_step(m);
  init_structure_label(m);
  init_structure(types.structure);
  init_properties(m);
  init_calculation(m);
  init_compound(types.compound);
  init_reaction(types.reaction);
  init_credentials(m);

  // Basic Database Classes
  init_collection(types.collection);
  init_manager(types.manager);
  init_layout(m);
}
