Changelog
=========

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
 
