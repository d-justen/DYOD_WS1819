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

Table::Table(const uint32_t chunk_size) : _chunks{std::make_shared<Chunk>()}, _chunk_size(chunk_size) {}

void Table::add_column_definition(const std::string& name, const std::string& type) {
  // Implementation goes here
}

void Table::add_column(const std::string& name, const std::string& type) {
  _column_names.push_back(name);
  _types.push_back(type);
  for (uint32_t entry = 0; entry < chunk_count(); ++entry) {
    get_chunk(ChunkID{entry}).add_segment(make_shared_by_data_type<BaseSegment, ValueSegment>(type));
  }
}

void Table::append(std::vector<AllTypeVariant> values) {
  if (_chunks.back()->size() >= chunk_size()) {
    _chunks.push_back(std::make_shared<Chunk>());
    for (const auto& type : _types) {
      _chunks.back()->add_segment(make_shared_by_data_type<BaseSegment, ValueSegment>(type));
    }
  }
  _chunks.back()->append(values);
}

uint16_t Table::column_count() const { return _chunks[0]->column_count(); }

void Table::create_new_chunk() {
  // Implementation goes here
}

uint64_t Table::row_count() const { return (chunk_count() - 1) * _chunk_size + _chunks.back()->size(); }

ChunkID Table::chunk_count() const { return ChunkID{_chunks.size()}; }

ColumnID Table::column_id_by_name(const std::string& column_name) const {
  for (int column = 0; column < column_count(); ++column) {
    if (_column_names[column] == column_name) {
      return ColumnID{column};
    }
  }
  throw std::runtime_error("Column not found");
}

uint32_t Table::chunk_size() const { return _chunk_size; }

const std::vector<std::string>& Table::column_names() const { return _column_names; }

const std::string& Table::column_name(ColumnID column_id) const { return _column_names[column_id]; }

const std::string& Table::column_type(ColumnID column_id) const { return _types[column_id]; }

Chunk& Table::get_chunk(ChunkID chunk_id) { return *_chunks[chunk_id]; }

const Chunk& Table::get_chunk(ChunkID chunk_id) const { return *_chunks[chunk_id]; }

void Table::compress_chunk(ChunkID chunk_id) {
  static std::mutex mutex;
  std::lock_guard<std::mutex> lock(mutex);

  auto new_chunk = std::make_shared<Chunk>();
  for (size_t index = 0; index < _chunks[chunk_id]->column_count(); ++index) {
    new_chunk->add_segment(make_shared_by_data_type<BaseSegment, DictionarySegment>(
        column_type(ColumnID{index}), _chunks[chunk_id]->get_segment(ColumnID{index})));
  }
  _chunks[chunk_id] = new_chunk;
}

void emplace_chunk(Chunk chunk) {
  // Implementation goes here
}

}  // namespace opossum
