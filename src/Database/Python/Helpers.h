/**
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef INCLUDE_DATABASE_PYBIND_HELPERS_H
#define INCLUDE_DATABASE_PYBIND_HELPERS_H

#include <boost/optional.hpp>

template<typename CppClass, typename PyClass, typename Has, typename Get, typename Set, typename Clear>
void def_optional_property(PyClass& c, const char* name, Has&& has, Get&& get, Set&& set, Clear&& clear, const char* docstring) {
  using PropertyType = decltype(get(std::declval<CppClass>()));
  // clang-format off
  c.def_property(
    name,
    [=](const CppClass& cls) -> boost::optional<PropertyType> {
      if(has(cls)) {
        return get(cls);
      }

      return boost::none;
    },
    [=](CppClass& cls, boost::optional<PropertyType> property) -> void {
      if(property) {
        set(cls, property.value());
      } else {
        clear(cls);
      }
    },
    docstring
  );
  // clang-format on
}

template<typename CppClass, typename PyClass, typename PartialGet, typename Set, typename Clear>
void def_optional_property(PyClass& c, const char* name, PartialGet&& partialGet, Set&& set, Clear&& clear,
                           const char* docstring) {
  using OptionalProperty = decltype(partialGet(std::declval<CppClass>()));
  // clang-format off
  c.def_property(
    name,
    partialGet,
    [=](CppClass& cls, OptionalProperty property) -> void {
      if(property) {
        set(cls, property.value());
      } else {
        clear(cls);
      }
    },
    docstring
  );
  // clang-format on
}

#endif
