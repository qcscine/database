/**
 * @file ModelPython.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include <Database/Objects/Model.h>
#include <Utils/Pybind.h>
#include <Utils/Settings.h>
#include <pybind11/operators.h>
#include <boost/variant/get.hpp>
#include <boost/variant/variant.hpp>

using namespace Scine::Database;

void init_model(pybind11::module& m) {
  pybind11::class_<Model> model(m, "Model",
                                R"delim(
      Data class for the model used for a quantum chemical calculation. Data is
      free-form, stored as strings. These should generally be lowercase. Three
      special values exist:

      - ``none``: The parameter is is off or not applicable. For instance,
        ``solvation`` may be ``none`` to indicate a calculation is carried out
        in the gas phase. Empty strings are also interpreted as ``none``.
      - ``any``: The parameter applies, but is unspecified and may be
        determined automatically. For instance, leaving the ``program``
        parameter empty may let any program supporting the method execute the
        calculation.

      Relationships between parameters of the model are defined by downstream
      code.

      :note: Manipulations of members of this class do not change database values.

      :example:
      >>> model = Model(method_family="dft", method="pbe2", basis_set="def2-tzvp")
      >>> model.method_family
      'dft'
      >>> model.spin_mode
      'any'
    )delim");
  model.def(pybind11::init<std::string, std::string, std::string, std::string>(), pybind11::arg("method_family"),
            pybind11::arg("method"), pybind11::arg("basis_set"), pybind11::arg("spin_mode"));
  model.def(pybind11::init<std::string, std::string, std::string>(), pybind11::arg("method_family"),
            pybind11::arg("method"), pybind11::arg("basis_set"));
  model.def_readwrite("spin_mode", &Model::spinMode, "Spin mode of the calculation: ``restricted``/``unrestricted``/``any``");
  model.def_readwrite("basis_set", &Model::basisSet,
                      "Basis set, e.g.: ``def2-svp`` or an empty string for semiempirical methods");
  model.def_readwrite("method", &Model::method, "Specific method in family of methods, e.g. ``pbe0`` in ``dft`` family");
  model.def_readwrite("method_family", &Model::methodFamily,
                      "Overarching method category for family of methods, e.g. ``cc`` for ``ccsd``, ``ccsd(t)`` etc.");
  model.def_readwrite("program", &Model::program, "Software program executing the calculation");
  model.def_readwrite("version", &Model::version, "Version of the software program executing the calculation");
  model.def_readwrite("solvation", &Model::solvation, "Implicit solvation model");
  model.def_readwrite("solvent", &Model::solvent, "Solvent of solvation model");
  model.def_readwrite("embedding", &Model::embedding, "QM/MM embedding system boundaries, in program-specific format");
  model.def_readwrite("periodic_boundaries", &Model::periodicBoundaries, "Periodic boundary conditions");
  model.def_readwrite("external_field", &Model::externalField, "Magnetic field boundary conditions");
  model.def_property(
      "temperature", [&](Model& self) { return self.temperature; },
      [&](Model& self, boost::variant<std::string, double> temp) {
        if (temp.type() == typeid(double)) {
          self.temperature = std::to_string(boost::get<double>(temp));
        }
        else {
          self.temperature = boost::get<std::string>(temp);
        }
      },
      "Temperature for thermodynamical property calculations");
  model.def_property(
      "pressure", [&](Model& self) { return self.pressure; },
      [&](Model& self, boost::variant<std::string, double> temp) {
        if (temp.type() == typeid(double)) {
          self.pressure = std::to_string(boost::get<double>(temp));
        }
        else {
          self.pressure = boost::get<std::string>(temp);
        }
      },
      "Pressure for thermodynamical property calculations");
  model.def_property(
      "electronic_temperature", [&](Model& self) { return self.electronicTemperature; },
      [&](Model& self, boost::variant<std::string, double> temp) {
        if (temp.type() == typeid(double)) {
          self.electronicTemperature = std::to_string(boost::get<double>(temp));
        }
        else {
          self.electronicTemperature = boost::get<std::string>(temp);
        }
      },
      "Temperature of electrons moving around nuclei");
  model.def("complete_model", &Model::completeModel, pybind11::arg("settings"),
            R"delim(
      Complete ``any`` fields by ``Settings`` values at matching keys

      Completes indeterminate fields by values from a ``Settings`` instance by
      matching their keys. Skips the ``method_family``, ``program``, and
      ``version`` fields.

      :raises RuntimeError: If values in the ``Settings`` instance at matching
        keys differ, or if the ``Settings`` instance is missing a key that is
        not ``none`` in the ``Model``.
    )delim");
  model.def("complete_settings", &Model::completeSettings, pybind11::arg("settings"),
            R"delim(
      Set ``Settings`` fields from non-``any`` fields in the database.

      Overwrites ``Settings`` key values matching fields in this ``Model`` unless
      they are ``any``. Skips the ``method_family``, ``program`` and
      ``version`` fields.

      :param settings: ``Settings`` instance to complete
      :raises RuntimeError: If ``settings`` lacks a non-``none`` field key
    )delim");
  model.def("__str__", &Model::getStringRepresentation,
            "Gives a simple string representation of all model fields with "
            "each field in one separate line.");
  model.def("__copy__", [](const Model& self) -> Model { return Model(self); });
  model.def("__deepcopy__", [](const Model& self, pybind11::dict /* memo */) -> Model { return Model(self); });
  // Comparison operators
  model.def(pybind11::self == pybind11::self);
  model.def(pybind11::self != pybind11::self);
  model.def(pybind11::pickle(
      [](const Model& m) { // __getstate__
        /* Return a tuple that fully encodes the state of the object */
        return pybind11::make_tuple(m.methodFamily, m.method, m.basisSet, m.spinMode, m.program, m.version, m.solvation,
                                    m.solvent, m.embedding, m.periodicBoundaries, m.externalField, m.temperature,
                                    m.electronicTemperature, m.pressure);
      },
      [](pybind11::tuple t) { // __setstate__
        if (t.size() != 14)
          throw std::runtime_error("Invalid state for Model!");

        /* Create a new C++ instance */
        Model m(t[0].cast<std::string>(), t[1].cast<std::string>(), t[2].cast<std::string>());
        m.spinMode = t[3].cast<std::string>();
        m.program = t[4].cast<std::string>();
        m.version = t[5].cast<std::string>();
        m.solvation = t[6].cast<std::string>();
        m.solvent = t[7].cast<std::string>();
        m.embedding = t[8].cast<std::string>();
        m.periodicBoundaries = t[9].cast<std::string>();
        m.externalField = t[10].cast<std::string>();
        m.temperature = t[11].cast<std::string>();
        m.electronicTemperature = t[12].cast<std::string>();
        m.pressure = t[13].cast<std::string>();

        return m;
      }));
}
