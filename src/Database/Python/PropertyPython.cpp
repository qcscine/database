/**
 * @file PropertyPython.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include "Database/Exceptions.h"
#include "Helpers.h"
#include "ObjectsVariant.h"
#include <Database/Objects/BoolProperty.h>
#include <Database/Objects/DenseMatrixProperty.h>
#include <Database/Objects/Model.h>
#include <Database/Objects/NumberProperty.h>
#include <Database/Objects/Property.h>
#include <Database/Objects/SparseMatrixProperty.h>
#include <Database/Objects/StringProperty.h>
#include <Database/Objects/VectorProperty.h>
#include <pybind11/eigen.h>
#include <Eigen/Dense>
#include <Eigen/Sparse>

namespace {

template<typename DerivedProperty, typename T>
void def_derived_property(const char* name, pybind11::module& m) {
  using namespace Scine;
  using namespace Database;

  // Sometimes the parameter is passed bare (bool and double), otherwise const&
  using DataParameterType = typename std::conditional<std::is_fundamental<T>::value, T, const T&>::type;

  pybind11::class_<DerivedProperty, Property> derived(m, name);
  derived.def(pybind11::init<>());
  derived.def(pybind11::init<ID>());
  derived.def(pybind11::init<ID, const Property::CollectionPtr&>());

  derived.def_static("make",
                     pybind11::overload_cast<const std::string&, const Model&, DataParameterType, const Object::CollectionPtr&>(
                         &DerivedProperty::create),
                     pybind11::arg("name"), pybind11::arg("model"), pybind11::arg("data"), pybind11::arg("collection"),
                     R"delim(
      Create a new property in a collection

      :param name: The name of the property
      :param model: The model used to calculate the property
      :param data: The property data itself
      :param collection: The collection to write the property into
    )delim");

  derived.def_static(
      "make",
      pybind11::overload_cast<const std::string&, const Model&, DataParameterType, const ID&, const ID&, const Object::CollectionPtr&>(
          &DerivedProperty::create),
      pybind11::arg("name"), pybind11::arg("model"), pybind11::arg("data"), pybind11::arg("structure_id"),
      pybind11::arg("calculation_id"), pybind11::arg("collection"),
      R"delim(
      Create a new property in a collection

      :param name: The name of the property
      :param model: The model used to calculate the property
      :param data: The property data itself
      :param structure_id: The structure to which the property is related to
      :param calculation_id: The calculation the property was calculated in
      :param collection: The collection to write the property into
    )delim");

  derived.def("create", pybind11::overload_cast<const Model&, const std::string&, DataParameterType>(&DerivedProperty::create),
              pybind11::arg("model"), pybind11::arg("property_name"), pybind11::arg("data"));
  derived.def("create",
              pybind11::overload_cast<const Model&, const std::string&, const ID&, const ID&, DataParameterType>(
                  &DerivedProperty::create),
              pybind11::arg("model"), pybind11::arg("property_name"), pybind11::arg("structure"),
              pybind11::arg("calculation"), pybind11::arg("data"));

  derived.def_property("data", &DerivedProperty::getData, &DerivedProperty::setData);
  derived.def("get_data", &DerivedProperty::getData);
  derived.def("set_data", &DerivedProperty::setData, pybind11::arg("data"));
}

} // namespace

void init_properties(pybind11::module& m) {
  using namespace Scine;
  using namespace Database;

  /*=======================*
   *  Property Base Class
   *=======================*/
  pybind11::class_<Property, Object> property(m, "Property",
                                              R"delim(
      Base class for datatype-differentiated derived property classes

      A ``Property`` represents named calculated properties of structures.

      :note: This class is, data-wise, merely a database pointer. Nearly all
        methods or properties invoke a database operation. No data is cached. Be
        wary of performance pitfalls in failing to cache reused large objects.

      Relationships with other database objects:
      - ``Calculation``: Calculations can generate new properties during
        execution and reference these in their result data.
      - ``Structure``: Properties are usually calculated on the basis of
        structures and refer back to these.

      Properties consist of a name identifier and a model that describes how
      the property data was calculated. Optionally, a free-form comment string
      can be added.

      :example:
      >>> properties = manager.get_collection("properties")
      >>> model = Model("dft", "pbe", "def2-svp")
      >>> p = NumberProperty.make("energy", model, 42.0, properties)
      >>> isinstance(p, Property)  # NumberProperty is a derived Property
      True
      >>> p.name
      'energy'
      >>> p.comment = "answer to life, the universe and everything"

      Properties can link to a ``Calculation`` and ``Structure`` instance,
      usually referring back to the structure the property was calculated for
      and the calculation that performed that calculation.

      :example:
      >>> calculations = manager.get_collection("calculations")
      >>> model = Model("dft", "pbe", "def2-svp")
      >>> job = Job("single_point")
      >>> bogus_id = ID()  # Bogus ID referencing a non-existent structure
      >>> calculation = Calculation.make(model, job, [bogus_id], calculations)
      >>> properties = manager.get_collection("properties")
      >>> p = NumberProperty.make("energy", model, 42.0, properties)
      >>> p.calculation_id is None
      True
      >>> p.calculation_id = calculation.id()
      >>> p.calculation_id is None
      False
      >>> p.structure_id is None  # We haven't linked a structure
      True

      Derived property classes have a ``data`` member property whose type
      depends on the kind of data stored in the database for it.
    )delim");
  property.def(pybind11::init<ID>(), "Constructs an unlinked property. Cannot use any methods besides ``link``.");
  property.def(pybind11::init<ID, const Property::CollectionPtr&>(), pybind11::arg("id"), pybind11::arg("collection"),
               R"delim(
      Construct a Property representing an existing entry in a passed collection

      :param id: Id of the property in the database
      :param collection: The collection the property is in
    )delim");

  property.def_property("name", &Property::getPropertyName, &Property::setPropertyName, "Name of the property");

  property.def("get_property_name", &Property::getPropertyName);
  property.def("set_property_name", &Property::setPropertyName, pybind11::arg("property_name"));

  property.def_property("model", &Property::getModel, &Property::setModel, "Model used to calculate the data");
  property.def("get_model", &Property::getModel);
  property.def("set_model", &Property::setModel, pybind11::arg("model"));

  def_optional_property<Property>(
      property, "structure_id", std::mem_fn(&Property::hasStructure), [](const Property& p) { return p.getStructure(); },
      std::mem_fn(&Property::setStructure), std::mem_fn(&Property::clearStructure),
      "ID referencing the structure from which this property was calculated. Can be ``None``");

  property.def("has_structure", &Property::hasStructure);
  property.def("set_structure", &Property::setStructure, pybind11::arg("structure"));
  property.def("get_structure", [](const Property& p) { return p.getStructure(); });
  property.def("clear_structure", &Property::clearStructure);

  def_optional_property<Property>(
      property, "calculation_id", std::mem_fn(&Property::hasCalculation),
      [](const Property& p) { return p.getCalculation(); }, std::mem_fn(&Property::setCalculation),
      std::mem_fn(&Property::clearCalculation),
      "ID referencing the calculation where this property was calculated. Can be ``None``");

  property.def("has_calculation", &Property::hasCalculation);
  property.def("set_calculation", &Property::setCalculation, pybind11::arg("calculation"));
  property.def("get_calculation", pybind11::overload_cast<>(&Property::getCalculation, pybind11::const_));
  property.def("clear_calculation", &Property::clearCalculation);

  def_optional_property<Property>(property, "comment", std::mem_fn(&Property::hasComment), std::mem_fn(&Property::getComment),
                                  std::mem_fn(&Property::setComment), std::mem_fn(&Property::clearComment),
                                  "Free-form string comment on the property. Can be ``None``.");

  property.def("has_comment", &Property::hasComment);
  property.def("set_comment", &Property::setComment, pybind11::arg("comment"));
  property.def("get_comment", &Property::getComment);
  property.def("clear_comment", &Property::clearComment);

  /* Derived property classes */
  def_derived_property<BoolProperty, bool>("BoolProperty", m);
  def_derived_property<NumberProperty, double>("NumberProperty", m);
  def_derived_property<StringProperty, std::string>("StringProperty", m);
  def_derived_property<VectorProperty, Eigen::VectorXd>("VectorProperty", m);
  def_derived_property<DenseMatrixProperty, Eigen::MatrixXd>("DenseMatrixProperty", m);
  def_derived_property<SparseMatrixProperty, Eigen::SparseMatrix<double>>("SparseMatrixProperty", m);

  /*==========================*
   *       Downcasting
   *==========================*/
  property.def(
      "get_derived",
      [](const Property& p) -> PropertiesVariant {
        using bsoncxx::builder::stream::document;
        using bsoncxx::builder::stream::finalize;

        if (!p.hasLink()) {
          throw Exceptions::MissingLinkedCollectionException();
        }

        auto selection = document{} << "_id" << p.id().bsoncxx() << finalize;
        auto optional = p.collection()->mongocxx().find_one(selection.view());
        if (!optional) {
          throw Exceptions::IDNotFoundException();
        }
        auto doc = optional.value().view();
        auto objecttype = doc["_objecttype"].get_utf8().value.to_string();

        if (objecttype != Property::objecttype) {
          throw Exceptions::ObjectTypeMismatchException();
        }

        const auto propertytype = doc["_propertytype"].get_utf8().value;
        return fetchProperty<PropertiesVariant>(propertytype, *p.collection(), p.id());
      },
      R"delim(
      Fetch the derived property for this base class instance

      Instantiates the right derived property type referring to the same
      database object.

      :example:
      >>> properties = manager.get_collection("properties")
      >>> model = Model("dft", "pbe", "def2-svp")
      >>> derived = NumberProperty.make("energy", model, 1.0, properties)
      >>> property = Property(derived.id(), properties)
      >>> derived_again = property.get_derived()
      >>> isinstance(derived_again, NumberProperty)
      True
      >>> derived.id() == property.id() == derived_again.id()
      True
      >>> derived_again.data
      1.0
    )delim");
}
