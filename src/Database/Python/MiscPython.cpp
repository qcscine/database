/**
 * @file MiscPython.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Includes */
#include <Database/Misc.h>
#include <Utils/Pybind.h>

using namespace Scine::Database;

void init_misc(pybind11::module& m) {
  m.def("levenshtein_distance",
        pybind11::overload_cast<const char*, const char*, unsigned int, unsigned int, unsigned int>(&Misc::levenshtein),
        pybind11::arg("a"), pybind11::arg("b"), pybind11::arg("insert_cost") = 1, pybind11::arg("delete_cost") = 1,
        pybind11::arg("replace_cost") = 1);
}
