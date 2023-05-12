__copyright__ = """This file is part of SCINE Database.
This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

import sys
from dev.conan.base import ScineConan


class ScineDatabaseConan(ScineConan):
    name = "scine_database"
    version = "1.2.0"
    url = "https://github.com/qcscine/database"
    description = """
The SCINE Database is a database wrapper for a MongoDB encoding reaction
networks."""
    options = {
        "shared": [True, False],
        "python": [True, False],
        "tests": [True, False],
        "coverage": [True, False],
        "microarch": ["none", "detect"],
        "test_ip": "ANY",
        "python_version": "ANY"
    }
    python_version_string = str(sys.version_info.major) + \
                    "." + str(sys.version_info.minor)
    default_options = {
        "shared": True,
        "python": False,
        "tests": False,
        "coverage": False,
        "microarch": "none",
        "test_ip": "localhost",
        "python_version": python_version_string
    }
    exports = "dev/conan/*.py"
    exports_sources = [
        "dev/cmake/*",
        "src/*",
        "CMakeLists.txt",
        "README.rst",
        "LICENSE.txt",
        "dev/conan/hook.cmake",
        "dev/conan/glue/*"
    ]
    requires = [
        "mongo-cxx-driver/3.6.0",
        "eigen/[~=3.3.7]",
        "scine_utilities/8.0.0"
    ]
    cmake_name = "Database"
    cmake_definitions = {
        "TEST_MONGO_DB_IP": lambda self: self.options.test_ip
    }

    def package_id(self):
        del self.info.options.test_ip
        super().package_id()
