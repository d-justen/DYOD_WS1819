#include <iomanip>
#include <iterator>
#include <limits>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "base_segment.hpp"
#include "chunk.hpp"

#include "utils/assert.hpp"

namespace opossum {

void Chunk::add_segment(std::shared_ptr<BaseSegment> segment) {
  _segments.push_back(segment);
}

void Chunk::append(const std::vector<AllTypeVariant>& values) {
  DebugAssert(values.size() == _segments.size(), "Value count doesn't match.");
  for (size_t i = 0; i<_segments.size(); i++) {
    _segments[i]->append(values[i]);
  }
}

std::shared_ptr<BaseSegment> Chunk::get_segment(ColumnID column_id) const {
  return _segments.at(column_id);
}

uint16_t Chunk::column_count() const {
  return _segments.size();
}

uint32_t Chunk::size() const {
  return column_count() > 0 ? _segments[0]->size() : 0;
}

}  // namespace opossum
