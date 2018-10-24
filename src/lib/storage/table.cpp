#include "table.hpp"

#include <algorithm>
#include <iomanip>
#include <limits>
#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "value_segment.hpp"

#include "resolve_type.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

void Table::add_column(const std::string& name, const std::string& type) {
  _column_names.push_back(name);
  _column_types.push_back(type);

  if (_chunks.size() == 0) {
    _chunks.push_back(std::make_shared<Chunk>());
  }

  for (size_t i=0; i<_chunks.size(); i++) {
    _chunks[i]->add_segment(make_shared_by_data_type<BaseSegment, ValueSegment>(type));
  }
}

void Table::append(std::vector<AllTypeVariant> values) {
  if (_chunks.back()->size() < _max_chunk_size) {
    _chunks.back()->append(values);
  }
  else {
    auto chunk = std::make_shared<Chunk>();
    
    for (size_t i = 0; i < values.size(); i++) {
      chunk->add_segment(make_shared_by_data_type<BaseSegment, ValueSegment>(_column_types[i]));
    }
    
    chunk->append(values);
    _chunks.push_back(chunk);
  }
}

uint16_t Table::column_count() const {
  return _chunks.size() > 0 ? _chunks[0]->column_count() : 0;
}

uint64_t Table::row_count() const {
  return _chunks.size() > 0 ? _chunks.back()->size() + ((_chunks.size() - 1) * _max_chunk_size) : 0;
}

ChunkID Table::chunk_count() const {
  return ChunkID{_chunks.size()};
}

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  for (size_t i = 0; i < _column_names.size(); i++) {
    if (_column_names[i] == column_name) {
      return ColumnID{i};
    }
  }
  throw std::exception();
}

uint32_t Table::chunk_size() const {
  return _max_chunk_size;
}

const std::vector<std::string>& Table::column_names() const {
  return _column_names;
}

const std::string& Table::column_name(ColumnID column_id) const {
  return _column_names[column_id];
}

const std::string& Table::column_type(ColumnID column_id) const {
  return _column_types[column_id];
}

Chunk& Table::get_chunk(ChunkID chunk_id) { return *_chunks.at(chunk_id); }

const Chunk& Table::get_chunk(ChunkID chunk_id) const { return *_chunks.at(chunk_id); }

}  // namespace opossum
