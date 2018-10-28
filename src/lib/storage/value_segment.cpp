#include "value_segment.hpp"

#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "type_cast.hpp"
#include "utils/assert.hpp"
#include "utils/performance_warning.hpp"

namespace opossum {

template <typename T>
const AllTypeVariant ValueSegment<T>::operator[](const size_t offset) const {
  PerformanceWarning("operator[] used");
  return _segment.at(offset);
}

template <typename T>
void ValueSegment<T>::append(const AllTypeVariant& val) {
  _segment.push_back(type_cast<T>(val));
}

template <typename T>
size_t ValueSegment<T>::size() const {
  // Implementation goes here
  return _segment.size();
}

template <typename T>
const std::vector<T>& ValueSegment<T>::values() const {
  return _segment;
}

EXPLICITLY_INSTANTIATE_DATA_TYPES(ValueSegment);

}  // namespace opossum
