Changelog
=========

Release 1.4.0
-------------

Changes (DB schema changing)
 - Remove `MODEL_TRANSFORMATION` elementary step type

Additions (backwards compatible):
 - Exact type of stored integers in the database is checked before retrieval in case fields have been manipulated with other MongoDB applications
 - Projection options are added to almost all internal database queries to reduce the bandwidth requirements.
 - Added support for list-of-lists of integers as elements in value collections (calculation settings etc.).

Release 1.3.0
-------------

Changes (DB schema changing):
 - Add `restart_information` field to `Calculation`
 - Add `pressure` to `Model`
 - Add `MODEL_TRANSFORMATION` elementary step type
 - Change DB version to 1.2.0 and provide update script
 - Add additional structure labels for intermediate structures of calculations

Additions (backwards compatible):
 - Python package now includes pure Python code for more complex query utilities. This code is added in the modules
    - `queries`
    - `energy_query_functions`
    - `compound_and_flask_creation`
    - `concentration_query_functions`
    - `insert_concentration`.
   All of them are top level modules (`import scine_database.energy_query_functions`)
 - Added the possibility to configure the connection based on a custom URI instead of credentials.

Release 1.2.0
-------------

Changes (backwards compatible):
 - `getAggregate` per default recursively looks up the original structure
   in case the method calling Structure instance is a duplicate.
   This avoids adding duplicates to aggregates.
 - The methods related to duplicates were renamed, the old method calls
   are now deprecated.

Important Technical Changes:
 - Increase pickle support of Python bindings.

Bugfixes:
 - Corrected the 1.0.0 to 1.1.0 update script.
 - Corrected case sensitivity in model completion.
 - Corrected temperature setting completion for `none` case.

Release 1.1.0
-------------

Changes (require updates):
 - Structures are now sorted into `aggregates` which can be Compounds
   or Flasks
 - Update Reactions to connect Flasks and/or Compounds
    - `lhs` and `rhs` are expanded with a `type` attribute

Additions (backwards compatible):
 - Add atom index maps to elementary steps
 - Add Flasks, a new type of aggregate for stable non-bonded complexes
 - Add collection for Flasks
 - Add aggregate related functions to the Structure class

Deprecated:
 - All Compound related functions of the Structure class are now deprecated
   (they point to the new aggregate fields)


Release 1.0.0
-------------

Initial features:
 - Connection manager for MongoDB servers
 - Definition of specialized classes for data objects that 
   encode chemical reaction networks
 - All functionalities present in C++ and Python3 bindings

Initial classes:
 - Structure
 - Compound (aggregate of Structures)
 - ElementaryStep
 - Reaction (aggregate of ElementarySteps)
 - Property (Data class attached to Structures)
    - BoolProperty (bool/bool)
    - NumberProperty (double/float)
    - StringProperty (string/str) 
    - VectorProperty (Eigen::VectorXd/numpy.ndarray) 
    - DenseMatrixProperty (Eigen::MatrixXd/numpy.ndarray) 
    - SparseMatrixProperty (Eigen::SparseMatrix/scipy.sparse.csr\_matrix)
 - Calculation

Initial collections:
 - structures
 - compounds
 - elementary\_steps
 - reactions
 - properties
 - calculations
 
