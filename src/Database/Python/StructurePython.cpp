/**
 * @file StructurePython.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include "Helpers.h"
#include <Database/Collection.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/Object.h>
#include <Database/Objects/Structure.h>
#include <Utils/Geometry/AtomCollection.h>
#include <Utils/Geometry/ElementInfo.h>
#include <Utils/IO/ChemicalFileFormats/XyzStreamHandler.h>
#include <Utils/Pybind.h>
#include <Utils/Typenames.h>
#include <pybind11/eigen.h>
#include <fstream>
#include <map>
#include <string>

using namespace Scine::Utils;
using namespace Scine::Database;

namespace {

AtomCollection fromXyz(const std::string& xyz_file) {
  std::ifstream file(xyz_file);
  if (!file) {
    throw std::runtime_error("File: '" + xyz_file + "' not found!");
  }
  return XyzStreamHandler::read(file);
}

Structure makeHelper1(const std::string& xyz, const int charge, const int multiplicity, const Object::CollectionPtr& coll) {
  return Structure::create(fromXyz(xyz), charge, multiplicity, coll);
}

Structure makeHelper2(const std::string& xyz, const int charge, const int multiplicity, const Model& model,
                      const Structure::LABEL label, const Object::CollectionPtr& coll) {
  return Structure::create(fromXyz(xyz), charge, multiplicity, model, label, coll);
}

ID createHelper1(Structure& structure, const std::string& xyz_file, const int charge, const int multiplicity) {
  return structure.create(fromXyz(xyz_file), charge, multiplicity);
}

ID createHelper2(Structure& structure, const std::string& xyz_file, const int charge, const int multiplicity,
                 const Model& model, const Structure::LABEL label) {
  return structure.create(fromXyz(xyz_file), charge, multiplicity, model, label);
}

void setAtomsHelper(Structure& structure, const std::string& xyz_file) {
  structure.setAtoms(fromXyz(xyz_file));
}

} // namespace

void init_structure_label(pybind11::module& m) {
  pybind11::enum_<Structure::LABEL> label(m, "Label");
  label.value("NONE", Structure::LABEL::NONE);
  label.value("IRRELEVANT", Structure::LABEL::IRRELEVANT);
  label.value("DUPLICATE", Structure::LABEL::DUPLICATE);
  label.value("USER_GUESS", Structure::LABEL::USER_GUESS);
  label.value("USER_OPTIMIZED", Structure::LABEL::USER_OPTIMIZED);
  label.value("MINIMUM_GUESS", Structure::LABEL::MINIMUM_GUESS);
  label.value("MINIMUM_OPTIMIZED", Structure::LABEL::MINIMUM_OPTIMIZED);
  label.value("TS_GUESS", Structure::LABEL::TS_GUESS);
  label.value("TS_OPTIMIZED", Structure::LABEL::TS_OPTIMIZED);
  label.value("ELEMENTARY_STEP_GUESS", Structure::LABEL::ELEMENTARY_STEP_GUESS);
  label.value("ELEMENTARY_STEP_OPTIMIZED", Structure::LABEL::ELEMENTARY_STEP_OPTIMIZED);
  label.value("REACTIVE_COMPLEX_GUESS", Structure::LABEL::REACTIVE_COMPLEX_GUESS);
  label.value("REACTIVE_COMPLEX_SCANNED", Structure::LABEL::REACTIVE_COMPLEX_SCANNED);
  label.value("REACTIVE_COMPLEX_OPTIMIZED", Structure::LABEL::REACTIVE_COMPLEX_OPTIMIZED);
  label.value("SURFACE_GUESS", Structure::LABEL::SURFACE_GUESS);
  label.value("SURFACE_OPTIMIZED", Structure::LABEL::SURFACE_OPTIMIZED);
  label.value("SURFACE_ADSORPTION_GUESS", Structure::LABEL::SURFACE_ADSORPTION_GUESS);
}

void init_structure(pybind11::class_<Structure, Object>& structure) {
  structure.doc() = R"delim(
    Class referencing a molecular three-dimensional structure database object

    A Structure has the following relationships with other database objects:
    - ``Compound``: A compound is a collection of one or more ``Structure``
      instances. Typically, structures are grouped by the criterion that they
      represent the same molecule.
    - ``Calculation``: A calculation will usually require one or more
      ``Structure`` instances as input.
    - ``Property``: These classes are results of a ``Calculation`` that each
      represent a derived quantity describing a ``Structure``.

    A structure's data consists mainly of its three-dimensional structure, its
    charge, the spin multiplicity, and a label.

    :example:
    >>> collection = manager.get_collection("structures")
    >>> structure = Structure.make(atoms, charge=0, multiplicity=1, collection=collection)
    >>> structure.charge  # Note: Properties are database fetches, not cached data
    0
    >>> structure.multiplicity = 3
    >>> structure.label = Label.MINIMUM_GUESS

    What molecule a structure describes can be encoded by means of the
    graph-related methods. Graphs are stored as a free-form string-string map.
    The graph properties can be populated e.g. by SMILES or serializations of
    other molecular representations.

    :example:
    >>> collection = manager.get_collection("structures")
    >>> structure = Structure.make(atoms, charge=0, multiplicity=1, collection=collection)
    >>> structure.get_graphs()
    {}
    >>> structure.set_graph("smiles", "CO")
    >>> structure.get_graphs()
    {'smiles': 'CO'}

    A structure may be linked to a ``Compound``, which collects multiple
    structures, usually by the criterion that they are deemed to be describing
    the same molecule, though this is principally free-form.

    :example:
    >>> structures = manager.get_collection("structures")
    >>> compounds = manager.get_collection("compounds")
    >>> structure = Structure.make(atoms, charge=0, multiplicity=1, collection=structures)
    >>> structure.compound_id is None
    True
    >>> compound = Compound.make([structure.id()], compounds)
    >>> structure.compound_id is None
    True
    >>> structure.compound_id = compound.id()  # Only now are they fully connected
    >>> structure.compound_id is None
    False

    ``Properties`` derived from a structure are referenced directly in the
    database representation, and can be manipulated.

    :example:
    >>> structures = manager.get_collection("structures")
    >>> properties = manager.get_collection("properties")
    >>> structure = Structure.make(atoms, charge=0, multiplicity=1, collection=structures)
    >>> bigness = NumberProperty.make("bigness", structure.model, 2.5, properties)
    >>> structure.add_property("bigness", bigness.id())
    >>> structure.get_property("bigness") == bigness.id()
    True
  )delim";

  structure.def(pybind11::init<>(), R"delim(
    Empty-initialization

    Nearly all member functions called for an instance in this state will raise
    exceptions since no ID is present nor a collection linked. The only way to
    use this instance is by linking a collection and then creating a new
    structure in the database with the ``create`` family of instance methods.
    This pattern is disadvised though: Prefer using the ``make`` family of
    static methods to directly generate new database objects and receive object
    instances fully populated with ID and linked collection.

    :example:
    >>> s = Structure()  # no ID, no linked collection
    >>> s.comment
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    RuntimeError: Missing linked collection.
    >>> collection = manager.get_collection("structures")
    >>> s.link(collection)
    >>> s.comment  # s still doesn't refer to a database object
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    RuntimeError: The Object is missing an ID to be used in this context.
    >>> id = s.create(atoms, 0, 1)  # Generate a database instance
    >>> id == s.id()
    True
    >>> t = Structure.make(atoms, 0, 1, collection)  # Direct via static method
    >>> t.has_id() and t.has_link()
    True
    >>> t.id() == s.id()
    False
  )delim");
  structure.def(pybind11::init<const ID&>(), pybind11::arg("id"),
                R"delim(
      Partial-initialization to existing database object

      A member function initialized by id, but without a collection refers
      to an existing database object, but does not fully specify which one,
      since ID uniqueness constraints are collection-level in the database.
      Nearly all member functions will raise for an instance in this state.
      The instance must be linked to a collection before it can be used.

      :example:
      >>> collection = manager.get_collection("structures")
      >>> obj = Structure.make(atoms, 0, 1, collection)  # Generate a db object
      >>> s = Structure(obj.id())  # Instance referring existing id
      >>> s.charge
      Traceback (most recent call last):
        File "<stdin>", line 1, in <module>
      RuntimeError: Missing linked collection.
      >>> s.link(collection)
      >>> s.charge
      0
    )delim");
  structure.def(pybind11::init<const ID&, const Object::CollectionPtr&>(), pybind11::arg("id"), pybind11::arg("collection"),
                R"delim(
      Full initialization to existing database object

      :example:
      >>> collection = manager.get_collection("structures")
      >>> s = Structure.make(atoms, 0, 1, collection)  # Generate a db object
      >>> t = Structure(s.id(), collection)  # Full initialization
      >>> s.id() == t.id()  # Both instances refer to the same database object
      True
    )delim");

  structure.def_static(
      "make", pybind11::overload_cast<const AtomCollection&, int, int, const Object::CollectionPtr&>(&Structure::create),
      pybind11::arg("atoms"), pybind11::arg("charge"), pybind11::arg("multiplicity"), pybind11::arg("collection"));
  structure.def_static(
      "make",
      pybind11::overload_cast<const AtomCollection&, int, int, const Model&, Structure::LABEL, const Object::CollectionPtr&>(
          &Structure::create),
      pybind11::arg("atoms"), pybind11::arg("charge"), pybind11::arg("multiplicity"), pybind11::arg("model"),
      pybind11::arg("label"), pybind11::arg("collection"));
  structure.def_static("make", &makeHelper1, pybind11::arg("xyz_file"), pybind11::arg("charge"),
                       pybind11::arg("multiplicity"), pybind11::arg("collection"));
  structure.def_static("make", &makeHelper2, pybind11::arg("xyz_file"), pybind11::arg("charge"), pybind11::arg("multiplicity"),
                       pybind11::arg("model"), pybind11::arg("label"), pybind11::arg("collection"));

  structure.def("create", pybind11::overload_cast<const AtomCollection&, int, int>(&Structure::create),
                pybind11::arg("atoms"), pybind11::arg("charge"), pybind11::arg("multiplicity"));
  structure.def("create",
                pybind11::overload_cast<const AtomCollection&, int, int, const Model&, Structure::LABEL>(&Structure::create),
                pybind11::arg("atoms"), pybind11::arg("charge"), pybind11::arg("multiplicity"), pybind11::arg("model"),
                pybind11::arg("label"));
  structure.def("create", &createHelper1, pybind11::arg("xyz_file"), pybind11::arg("charge"), pybind11::arg("multiplicity"));
  structure.def("create", &createHelper2, pybind11::arg("xyz_file"), pybind11::arg("charge"),
                pybind11::arg("multiplicity"), pybind11::arg("model"), pybind11::arg("label"));

  structure.def("get_atoms", &Structure::getAtoms);
  structure.def("set_atoms", &Structure::setAtoms, pybind11::arg("atoms"));
  structure.def("set_atoms", &setAtomsHelper, pybind11::arg("xyz_file"), "Populate the atoms from an .xyz file");
  structure.def("has_atoms", &Structure::hasAtoms);
  structure.def("clear_atoms", &Structure::clearAtoms);

  structure.def("get_model", &Structure::getModel);
  structure.def("set_model", &Structure::setModel, pybind11::arg("model"));
  structure.def_property("model", &Structure::getModel, &Structure::setModel, "Model used to generate this structure");

  structure.def("get_label", &Structure::getLabel);
  structure.def("set_label", &Structure::setLabel, pybind11::arg("label"));
  structure.def_property("label", &Structure::getLabel, &Structure::setLabel, "Label describing how the structure was generated");

  structure.def("get_charge", &Structure::getCharge);
  structure.def("set_charge", &Structure::setCharge, pybind11::arg("charge"));
  structure.def_property("charge", &Structure::getCharge, &Structure::setCharge, "Overall molecular charge");

  structure.def("get_multiplicity", &Structure::getMultiplicity);
  structure.def("set_multiplicity", &Structure::setMultiplicity, pybind11::arg("multiplicity"));
  structure.def_property("multiplicity", &Structure::getMultiplicity, &Structure::setMultiplicity, "Spin multiplicity");

  structure.def("get_compound", &Structure::getCompound);
  structure.def("set_compound", &Structure::setCompound, pybind11::arg("compound"));
  structure.def("has_compound", &Structure::hasCompound);
  structure.def("clear_compound", &Structure::clearCompound);
  def_optional_property<Structure>(structure, "compound_id", std::mem_fn(&Structure::hasCompound),
                                   std::mem_fn(&Structure::getCompound), std::mem_fn(&Structure::setCompound),
                                   std::mem_fn(&Structure::clearCompound), "Linked compound id");

  structure.def("has_property", pybind11::overload_cast<const std::string&>(&Structure::hasProperty, pybind11::const_),
                pybind11::arg("key"));
  structure.def("has_property", pybind11::overload_cast<const ID&>(&Structure::hasProperty, pybind11::const_),
                pybind11::arg("id"));
  structure.def("get_property", &Structure::getProperty, pybind11::arg("key"));
  structure.def("set_property", &Structure::setProperty, pybind11::arg("key"), pybind11::arg("id"));
  structure.def("add_property", &Structure::addProperty, pybind11::arg("key"), pybind11::arg("id"));
  structure.def("remove_property", &Structure::removeProperty, pybind11::arg("key"), pybind11::arg("id"));
  structure.def("set_properties", &Structure::setProperties, pybind11::arg("key"), pybind11::arg("ids"));
  structure.def("get_properties", &Structure::getProperties, pybind11::arg("key"));
  structure.def("query_properties", &Structure::queryProperties, pybind11::arg("key"), pybind11::arg("model"),
                pybind11::arg("collection"));
  structure.def("has_properties", &Structure::hasProperties, pybind11::arg("key"));
  structure.def("clear_properties", &Structure::clearProperties, pybind11::arg("key"));
  structure.def("get_all_properties", &Structure::getAllProperties);
  structure.def("set_all_properties", &Structure::setAllProperties, pybind11::arg("properties"));
  structure.def("clear_all_properties", &Structure::clearAllProperties);

  structure.def("get_graph", &Structure::getGraph, pybind11::arg("key"));
  structure.def("set_graph", &Structure::setGraph, pybind11::arg("key"), pybind11::arg("graph"));
  structure.def("remove_graph", &Structure::removeGraph, pybind11::arg("key"));
  structure.def("has_graph", &Structure::hasGraph, pybind11::arg("key"));
  structure.def("has_graphs", &Structure::hasGraphs);
  structure.def("get_graphs", &Structure::getGraphs);
  structure.def("set_graphs", &Structure::setGraphs);
  structure.def("clear_graphs", &Structure::clearGraphs);

  structure.def("get_comment", &Structure::getComment);
  structure.def("set_comment", &Structure::setComment, pybind11::arg("comment"));
  structure.def("has_comment", &Structure::hasComment);
  structure.def("clear_comment", &Structure::clearComment);
  def_optional_property<Structure>(structure, "comment", std::mem_fn(&Structure::hasComment),
                                   std::mem_fn(&Structure::getComment), std::mem_fn(&Structure::setComment),
                                   std::mem_fn(&Structure::clearComment), "Free-form comment on the structure");
}
