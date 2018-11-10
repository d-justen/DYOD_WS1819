#pragma once

#include <memory>
#include <vector>

#include "base_attribute_vector.hpp"

namespace opossum {

class FittedAttributeVector : public BaseAttributeVector {
 public:
  FittedAttributeVector() = default;
  virtual ~FittedAttributeVector() = default;

  // returns the value id at a given position
  ValueID get(const size_t i) const override { return (*_values)[i]; }

  // sets the value id at a given position
  void set(const size_t i, const ValueID value_id) override { (*_values)[i] = value_id; }

  // returns the number of values
  size_t size() const override { return _values->size(); }

  // returns the width of biggest value id in bytes
  AttributeVectorWidth width() const override { return *std::max(_values->cbegin(), _values->cend()); }

 protected:
  std::shared_ptr<std::vector<ValueID>> _values;
};
}  // namespace opossum
