#pragma once

#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "all_type_variant.hpp"
#include "fitted_attribute_vector.hpp"
#include "type_cast.hpp"
#include "types.hpp"
#include "value_segment.hpp"

namespace opossum {

class BaseAttributeVector;
class BaseSegment;

// Even though ValueIDs do not have to use the full width of ValueID (uint32_t), this will also work for smaller ValueID
// types (uint8_t, uint16_t) since after a down-cast INVALID_VALUE_ID will look like their numeric_limit::max()
constexpr ValueID INVALID_VALUE_ID{std::numeric_limits<ValueID::base_type>::max()};

// Dictionary is a specific segment type that stores all its values in a vector
template <typename T>
class DictionarySegment : public BaseSegment {
 public:
  /**
   * Creates a Dictionary segment from a given value segment.
   */
  explicit DictionarySegment(const std::shared_ptr<BaseSegment>& base_segment)
      : _dictionary(std::make_shared<std::vector<T>>()) {
    const auto& value_segment = std::dynamic_pointer_cast<ValueSegment<T>>(base_segment);

    // Remove duplicates using a set
    std::set<T> deduplicated_set(value_segment->values().cbegin(), value_segment->values().cend());
    _dictionary->assign(deduplicated_set.cbegin(), deduplicated_set.cend());

    if (_dictionary->size() <= std::numeric_limits<uint8_t>::max()) {
      _attribute_vector = std::make_shared<FittedAttributeVector<uint8_t>>(base_segment->size());
    } else if (_dictionary->size() <= std::numeric_limits<uint16_t>::max()) {
      _attribute_vector = std::make_shared<FittedAttributeVector<uint16_t>>(base_segment->size());
    } else if (_dictionary->size() <= std::numeric_limits<uint32_t>::max()) {
      _attribute_vector = std::make_shared<FittedAttributeVector<uint32_t>>(base_segment->size());
    } else {
      _attribute_vector = std::make_shared<FittedAttributeVector<uint64_t>>(base_segment->size());
    }

    std::sort(_dictionary->begin(), _dictionary->end());

    // Insert references to dict into attribute vector
    for (size_t segment_idx = 0; segment_idx < base_segment->size(); ++segment_idx) {
      const auto& dict_it =
          std::lower_bound(_dictionary->cbegin(), _dictionary->cend(), type_cast<T>((*value_segment)[segment_idx]));

      if (dict_it != _dictionary->cend()) {
        _attribute_vector->set(segment_idx, ValueID{dict_it - _dictionary->cbegin()});
      }
    }
  }
  // SEMINAR INFORMATION: Since most of these methods depend on the template parameter, you will have to implement
  // the DictionarySegment in this file. Replace the method signatures with actual implementations.

  // return the value at a certain position. If you want to write efficient operators, back off!
  const AllTypeVariant operator[](const size_t i) const override { return get(i); }

  // return the value at a certain position.
  const T get(const size_t i) const { return (*_dictionary)[_attribute_vector->get(i)]; }

  // dictionary segments are immutable
  void append(const AllTypeVariant&) override { throw std::runtime_error("Dictionary segments are immutable"); }

  // returns an underlying dictionary
  std::shared_ptr<const std::vector<T>> dictionary() const { return _dictionary; }

  // returns an underlying data structure
  std::shared_ptr<const BaseAttributeVector> attribute_vector() const { return _attribute_vector; }

  // return the value represented by a given ValueID
  const T& value_by_value_id(ValueID value_id) const { return _dictionary[value_id]; }

  // returns the first value ID that refers to a value >= the search value
  // returns INVALID_VALUE_ID if all values are smaller than the search value
  ValueID lower_bound(T value) const {
    const auto& low = std::lower_bound(_dictionary->cbegin(), _dictionary->cend(), value);
    return (low == _dictionary->cend()) ? INVALID_VALUE_ID : ValueID{std::distance(_dictionary->cbegin(), low)};
  }

  // same as lower_bound(T), but accepts an AllTypeVariant
  ValueID lower_bound(const AllTypeVariant& value) const { return lower_bound(type_cast<T>(value)); }

  // returns the first value ID that refers to a value > the search value
  // returns INVALID_VALUE_ID if all values are smaller than or equal to the search value
  ValueID upper_bound(T value) const {
    const auto& up = std::upper_bound(_dictionary->cbegin(), _dictionary->cend(), value);
    return (up == _dictionary->cend()) ? INVALID_VALUE_ID : ValueID{std::distance(_dictionary->cbegin(), up)};
  }

  // same as upper_bound(T), but accepts an AllTypeVariant
  ValueID upper_bound(const AllTypeVariant& value) const { return upper_bound(type_cast<T>(value)); }

  // return the number of unique_values (dictionary entries)
  size_t unique_values_count() const { return _dictionary->size(); }

  // return the number of entries
  size_t size() const override { return _attribute_vector->size(); }

 protected:
  std::shared_ptr<std::vector<T>> _dictionary;
  std::shared_ptr<BaseAttributeVector> _attribute_vector;
};

}  // namespace opossum
