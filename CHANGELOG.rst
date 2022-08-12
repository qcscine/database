Changelog
=========

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
 
