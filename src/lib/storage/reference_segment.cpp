#include "reference_segment.hpp"

namespace opossum {

ReferenceSegment::ReferenceSegment(const std::shared_ptr<const Table> referenced_table,
                                   const ColumnID referenced_column_id, const std::shared_ptr<const PosList> pos)
    : _table(referenced_table), _column_id(referenced_column_id), _pos_list(pos) {}

const AllTypeVariant ReferenceSegment::operator[](const size_t i) const {
  DebugAssert(i < _pos_list->size(), "Index out of range.");
  const auto& row_id = (*_pos_list)[i];
  const auto& referenced_segment = _table->get_chunk(row_id.chunk_id).get_segment(_column_id);
  return (*referenced_segment)[row_id.chunk_offset];
}

size_t ReferenceSegment::size() const { return _pos_list->size(); }

const std::shared_ptr<const PosList> ReferenceSegment::pos_list() const { return _pos_list; }
const std::shared_ptr<const Table> ReferenceSegment::referenced_table() const { return _table; }

ColumnID ReferenceSegment::referenced_column_id() const { return _column_id; }

}  // namespace opossum
