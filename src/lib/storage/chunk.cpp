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

void Chunk::add_segment(std::shared_ptr<BaseSegment> segment) { _chunk.push_back(segment); }

void Chunk::append(const std::vector<AllTypeVariant>& values) {
  DebugAssert(values.size() == column_count(), "Wrong number of entries");
  for (uint16_t value = 0; value < column_count(); ++value) {
    _chunk.at(value)->append(value);
  }
}

std::shared_ptr<BaseSegment> Chunk::get_segment(ColumnID column_id) const { return _chunk.at(column_id); }

uint16_t Chunk::column_count() const { return _chunk.size(); }

uint32_t Chunk::size() const { return column_count() <= 0 ? 0 : _chunk.at(0)->size(); }

}  // namespace opossum
