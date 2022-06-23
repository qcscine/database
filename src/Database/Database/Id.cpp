/**
 * @file Id.cpp
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Laboratory of Physical Chemistry, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

/* Internal Includes*/
#include <Database/Id.h>
/* External Includes */
#include <bsoncxx/json.hpp>
#include <bsoncxx/oid.hpp>
#include <bsoncxx/types/value.hpp>
#include <iostream>

namespace Scine {
namespace Database {

ID::ID() : _oid(std::make_unique<bsoncxx::oid>()) {
}
ID::~ID() = default;
ID::ID(const ID& other) {
  this->_oid = std::make_unique<bsoncxx::oid>(*other._oid);
}
ID::ID(ID&& other) noexcept = default;
ID& ID::operator=(const ID& rhs) {
  this->_oid = std::make_unique<bsoncxx::oid>(*rhs._oid);
  return *this;
}
ID& ID::operator=(ID&& rhs) noexcept = default;

ID::ID(bsoncxx::oid base) : _oid(std::make_unique<bsoncxx::oid>(base)) {
}

ID::ID(std::string id) : _oid(std::make_unique<bsoncxx::oid>(id)) {
}

std::string ID::string() const {
  return this->_oid->to_string();
}

bsoncxx::oid ID::bsoncxx() const {
  return *(this->_oid);
}

bool ID::operator==(const ID& other) const {
  return this->bsoncxx() == other.bsoncxx();
}

bool ID::operator<=(const ID& other) const {
  return this->bsoncxx() <= other.bsoncxx();
}

bool ID::operator>=(const ID& other) const {
  return this->bsoncxx() >= other.bsoncxx();
}

bool ID::operator<(const ID& other) const {
  return this->bsoncxx() < other.bsoncxx();
}

bool ID::operator>(const ID& other) const {
  return this->bsoncxx() > other.bsoncxx();
}

bool ID::operator==(const bsoncxx::oid& other) const {
  return this->bsoncxx() == other;
}

} /* namespace Database */
} /* namespace Scine */
