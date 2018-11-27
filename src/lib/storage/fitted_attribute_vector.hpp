#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "base_attribute_vector.hpp"

namespace opossum {

template <class T>
class FittedAttributeVector : public BaseAttributeVector {
 public:
  explicit FittedAttributeVector(const size_t size) : _values(std::make_shared<std::vector<T>>(size)) {}
  virtual ~FittedAttributeVector() = default;

  // returns the value id at a given position
  ValueID get(const size_t i) const override { return ValueID{(*_values)[i]}; }

  // sets the value id at a given position
  void set(const size_t i, const ValueID value_id) override { (*_values)[i] = value_id; }

  // returns the number of values
  size_t size() const override { return _values->size(); }

  // returns the width of biggest value id in bytes
  AttributeVectorWidth width() const override {
    return static_cast<AttributeVectorWidth>(sizeof(*std::max(_values->cbegin(), _values->cend())));
  }

 protected:
  std::shared_ptr<std::vector<T>> _values;
};
}  // namespace opossum
