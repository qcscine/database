/**
 * @file Id.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef DATABASE_ID_H_
#define DATABASE_ID_H_

/* External Includes */
#include <memory>
#include <string>

namespace bsoncxx {
inline namespace v_noabi {
class oid;
}
} // namespace bsoncxx

namespace Scine {
namespace Database {

/**
 * @class ID ID.h
 * @brief A wrapper for the bsoncxx::oid in order to add
 *        some functionalities.
 */
class ID {
 public:
  /**
   * @brief Construct a new ID.
   */
  explicit ID();
  /// @brief Destructor.
  ~ID();
  /// @brief Copy Constructor.
  ID(const ID& other);
  /// @brief Move Constructor.
  ID(ID&& other) noexcept;
  /// @brief Assigment Operator.
  ID& operator=(const ID& rhs);
  /// @brief Move Assignment Operator.
  ID& operator=(ID&& rhs) noexcept;
  /**
   * @brief Construct a new ID object.
   * @param base A bsoncxx::types::value representing the base bsoncxx::oid object.
   */
  ID(bsoncxx::v_noabi::oid base);
  /**
   * @brief Getter for the bson representation of the OID.
   * @return bsoncxx::v_noabi::oid The bson representation of the OID.
   */
  bsoncxx::v_noabi::oid bsoncxx() const;
  /**
   * @brief Construct a new ID object.
   * @param id A the identifier in string form.
   */
  ID(std::string id);
  /**
   * @brief The string version of the id.
   * @return std::string Returns the string version of the ID.
   */
  std::string string() const;
  // @brief Comparsion.
  bool operator==(const ID& other) const;
  // @brief Comparsion.
  bool operator<=(const ID& other) const;
  // @brief Comparsion.
  bool operator>=(const ID& other) const;
  // @brief Comparsion.
  bool operator<(const ID& other) const;
  // @brief Comparsion.
  bool operator>(const ID& other) const;
  /**
   * @brief Comparion operator.
   * @return bool Returns true if he IDs are identical.
   */
  bool operator==(const bsoncxx::v_noabi::oid& other) const;

 private:
  std::unique_ptr<bsoncxx::v_noabi::oid> _oid;
};

} /* namespace Database */
} /* namespace Scine */

#endif /* DATABASE_ID_H_ */
