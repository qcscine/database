/**
 * @file Exceptions.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_EXCEPTIONS_H_
#define DATABASE_EXCEPTIONS_H_

/* External Includes */
#include <exception>

namespace Scine {
namespace Database {
namespace Exceptions {

/**
 * @class DuplicateIDException
 * @brief An exception to throw if a given ID is already present, and this is not expected.
 */
class DuplicateIDException : public std::exception {
 public:
  const char* what() const throw() {
    return "The Object to be added to the database has an ID that is already existing in the database.";
  }
};

/**
 * @class SelfDuplicateException
 * @brief An exception to throw if a given structure is a duplicate of itself.
 */
class SelfDuplicateException : public std::exception {
 public:
  const char* what() const throw() {
    return "The given structure is a duplicate of itself. This cannot be.";
  }
};

/**
 * @class MissingCredentialsException
 * @brief An exception to throw if no credentials are present to establish a connection to a database.
 */
class MissingCredentialsException : public std::exception {
 public:
  const char* what() const throw() {
    return "No credentials available.";
  }
};

/**
 * @class MissingLinkedCollectionException
 * @brief An exception to throw if a given object is expected to have a collection linked, but does not.
 */
class MissingLinkedCollectionException : public std::exception {
 public:
  const char* what() const throw() {
    return "Missing linked collection.";
  }
};

/**
 * @class DatabaseDisconnectedException Exceptions.h
 * @brief An exception to throw if no connection to a database is established.
 */
class DatabaseDisconnectedException : public std::exception {
 public:
  const char* what() const throw() {
    return "No connection to a database available.";
  }
};

/**
 * @class MissingCollectionException Exceptions.h
 * @brief An exception to throw if a collection is missing.
 */
class MissingCollectionException : public std::exception {
 public:
  const char* what() const throw() {
    return "The requested collection could not be found.";
  }
};

/**
 * @class MissingIDException Exceptions.h
 * @brief An exception to throw if an ID is missing.
 */
class MissingIDException : public std::exception {
 public:
  const char* what() const throw() {
    return "The Object is missing an ID to be used in this context.";
  }
};

/**
 * @class MissingTimestampException Exceptions.h
 * @brief An exception to throw if an object is missing a required timestamp.
 */
class MissingTimestampException : public std::exception {
 public:
  const char* what() const throw() {
    return "The Object is missing a requested timestamp, update from or add to the database first.";
  }
};

/**
 * @class IDNotFoundException Exceptions.h
 * @brief An exception to throw if an Object with a given ID could not be found.
 */
class IDNotFoundException : public std::exception {
 public:
  const char* what() const throw() {
    return "No Object with the given ID could be found.";
  }
};

/**
 * @class UnpopulatedObjectException Exceptions.h
 * @brief An exception to throw a DB object is not populated.
 */
class UnpopulatedObjectException : public std::exception {
 public:
  const char* what() const throw() {
    return "The Object is missing data in at least one required field. Action aborted.";
  }
};

/**
 * @class ObjectTypeMismatchException Exceptions.h
 * @brief An exception to throw if the Object type requested and found do not match.
 */
class ObjectTypeMismatchException : public std::exception {
 public:
  const char* what() const throw() {
    return "The Object type requested does not match the one in the database.";
  }
};

/**
 * @class MissingIdOrField Exceptions.h
 * @brief An exception to throw if the Object type requested and found do not match.
 */
class MissingIdOrField : public std::exception {
 public:
  const char* what() const throw() {
    return "The object with the given ID, or the requested field in it could not be found in the database.";
  }
};

/**
 * @class FieldException
 * @brief An exception to throw if a restriction placed on a particular field in the database is not fulfilled.
 */
class FieldException : public std::exception {
 public:
  const char* what() const throw() {
    return "The requested field did not match the specifications.";
  }
};

/**
 * @class VersionMismatch
 * @brief An exception to throw if the wrapper and server version mismatch.
 */
class VersionMismatch : public std::exception {
 public:
  const char* what() const throw() {
    return "The database was created with a version that is not supported by this wrapper.";
  }
};

} /* namespace Exceptions */
} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_EXCEPTIONS_H_ */
