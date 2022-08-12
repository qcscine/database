__copyright__ = """This code is licensed under the 3-clause BSD license.
Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.
See LICENSE.txt for details.
"""

import setuptools
from typing import List, Dict
from pathlib import Path
import os

# Read README.rst for the long description
with open("README.rst", "r") as fh:
    long_description = fh.read()


class EmptyListWithLength(list):
    """ Makes the wheel a binary distribution and platlib compliant. """

    def __len__(self):
        return 1


def find_stubs(package_name: str) -> List[str]:
    """ Find typing stub files in the package directory """
    stubs = []
    for root, dirs, files in os.walk(package_name):
        for file in files:
            if not file.endswith(".pyi"):
                continue

            path = os.path.join(root, file)
            stubs.append(path.replace(package_name + os.sep, '', 1))
    return stubs


def collect_data(pkg_name: str) -> Dict[str, List[str]]:
    """ Generates the package_data dict with stubs (if present) """
    package_data = {pkg_name: ["scine_database.*", "*.txt"@database_PY_DEPS@]}

    # Handle possibility of typing stubs present
    stubs = find_stubs(pkg_name)
    if len(stubs) > 0:
        # Typing marker file for PEP 561
        typed_filename = "py.typed"
        typed_file = Path(".") / pkg_name / typed_filename
        typed_file.touch()
        package_data[pkg_name].extend(stubs)
        package_data[pkg_name].append(typed_filename)

    return package_data


# Define the setup
setuptools.setup(
    name="scine_database",
    version="@Database_VERSION@",
    author="ETH Zurich, Laboratory for Physical Chemistry, Reiher Group",
    author_email="scine@phys.chem.ethz.ch",
    description="SCINE database utilities for chemical reaction networks",
    long_description=long_description,
    url="https://www.scine.ethz.ch",
    packages=["scine_database"],
    package_data=collect_data("scine_database"),
    nstall_requires=["scine_utilities"],
    classifiers=[
        "Programming Language :: Python",
        "Programming Language :: C++",
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: BSD License",
        "Natural Language :: English",
        "Topic :: Scientific/Engineering :: Chemistry"
    ],
    zip_safe=False,
    test_suite='pytest',
    tests_require=['pytest'],
    ext_modules=EmptyListWithLength()
)
