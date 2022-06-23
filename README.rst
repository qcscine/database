SCINE - Database
================

Introduction
------------

The SCINE database module is a database wrapper for a MongoDB encoding reaction networks.

License and Copyright Information
---------------------------------

For license and copyright information, see the file ``LICENSE.txt`` in this
directory.

Installation and Usage
----------------------

The following software packages are required in order to compile the SCINE Database:

- A C++ compiler supporting the C++17 standard (GCC at least 7.3.0 or later)
- CMake (at least version 3.9.0)
- Eigen3 (at least version 3.3.2 or later)
- MongoDB C++ Driver (at least version 3.4.0)
- Boost (recommended: version 1.65.0 or later)

Furthermore the database compilation will require the SCINE Utilities module,
and download it automatically if it is not present.

The SCINE database repository includes a Git submodule; either clone the repository
recursively::

    git clone --recurse-submodules https://github.com/qcscine/database.git

or initialize and update the submodules after a regular clone::

    git clone https://github.com/qcscine/database.git scine_database
    cd scine_database
    git submodule update --init

The SCINE database module can be built using a standard CMake/make setup::

    mkdir build
    cd build
    cmake -DSCINE_BUILD_PYTHON_BINDINGS=ON ..
    make
    make test
    make install

Note that the tests, by default, require a MongoDB to be running on the local host.
Alternatively the ``-DTEST_MONGO_DB_IP=XXX`` flag can be set in the CMake configure
step to route the test executable to another database.

For minimal usage examples please see the user manual provided in this repository
or check the latest online version on the `SCINE web page <https://scine.ethz.ch>`_.

How to Cite
-----------

When publishing results obtained with the SCINE database wrapper, please cite the corresponding
release as archived on `Zenodo <https://zenodo.org/>`_ (please use the DOI of the
respective release).

In addition, we kindly request you to cite the following article when using the SCINE database wrapper:
J. P. Unsleber, S. A. Grimmel, M. Reiher,
"Chemoton 2.0: Autonomous Exploration of Chemical Reaction Networks",
arXiv:2202.13011 [physics.chem-ph].

Support and Contact
-------------------

In case you should encounter problems or bugs, please write a short message
to scine@phys.chem.ethz.ch.

Third-Party Libraries Used
--------------------------

SCINE Database makes use of the following third-party libraries:

- `Boost <https://www.boost.org/>`_
- `Eigen <http://eigen.tuxfamily.org>`_
- `Google Test <https://github.com/google/googletest>`_
- `MongoDB C++ Driver <https://github.com/mongodb/mongo-cxx-driver>`_
- `pybind11 <https://github.com/pybind/pybind11>`_
