#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "abstract_operator.hpp"
#include "all_type_variant.hpp"
#include "storage/dictionary_segment.hpp"
#include "storage/reference_segment.hpp"
#include "storage/table.hpp"
#include "type_cast.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

class Table;

class TableScan : public AbstractOperator {
 public:
  TableScan(const std::shared_ptr<const AbstractOperator> in, ColumnID column_id, const ScanType scan_type,
            const AllTypeVariant search_value);

  ~TableScan() = default;

  ColumnID column_id() const;
  ScanType scan_type() const;
  const AllTypeVariant& search_value() const;

 protected:
  class BaseTableScanImpl {
   public:
    BaseTableScanImpl() = default;
    virtual ~BaseTableScanImpl() = default;
    BaseTableScanImpl(BaseTableScanImpl&&) = default;
    BaseTableScanImpl& operator=(BaseTableScanImpl&&) = default;

    virtual std::shared_ptr<const Table> on_execute(const TableScan& table_scan) = 0;
  };

  template <typename T>
  class TableScanImpl : public BaseTableScanImpl {
   public:
    TableScanImpl() = default;
    ~TableScanImpl() = default;

   protected:
    template <typename U>
    bool compare(const U& lhs, const U& rhs, ScanType scan_type) {
      switch (scan_type) {
        case ScanType::OpEquals:
          return lhs == rhs;
          break;
        case ScanType::OpGreaterThan:
          return lhs > rhs;
          break;
        case ScanType::OpGreaterThanEquals:
          return lhs >= rhs;
          break;
        case ScanType::OpLessThan:
          return lhs < rhs;
          break;
        case ScanType::OpLessThanEquals:
          return lhs <= rhs;
          break;
        case ScanType::OpNotEquals:
          return lhs != rhs;
          break;
        default:
          return false;
          break;
      }
    }

    std::shared_ptr<Table> create_result_table(const TableScan& table_scan, uint64_t result_size,
                                               const std::shared_ptr<PosList> pos_list) {
      const auto& result_table = std::make_shared<Table>(result_size);

      for (ColumnID segment_index = ColumnID{0}; segment_index < table_scan._input_table_left()->column_count();
           ++segment_index) {
        const auto& column = std::dynamic_pointer_cast<ReferenceSegment>(
            table_scan._input_table_left()->get_chunk(ChunkID{0}).get_segment(segment_index));
        std::shared_ptr<ReferenceSegment> reference_segment;

        if (column) {
          reference_segment = std::make_shared<ReferenceSegment>(column->referenced_table(), segment_index, pos_list);
        } else {
          reference_segment =
              std::make_shared<ReferenceSegment>(table_scan._input_table_left(), segment_index, pos_list);
        }
        result_table->get_chunk(ChunkID{0}).add_segment(reference_segment);
        result_table->add_column_definition(table_scan._input_table_left()->column_name(segment_index),
                                            table_scan._input_table_left()->column_type(segment_index));
      }
      return result_table;
    }

    std::shared_ptr<const Table> on_execute(const TableScan& table_scan) {
      const auto& pos_list = std::make_shared<PosList>();
      uint64_t result_size = 0;

      for (ChunkID chunk_index = ChunkID{0}; chunk_index < table_scan._input_table_left()->chunk_count();
           ++chunk_index) {
        const auto& segment =
            table_scan._input_table_left()->get_chunk(chunk_index).get_segment(table_scan.column_id());
        // DebugAssert(std::is_same<segment.type_cast<T>(table_scan._search_value), "Types cannot be compared");

        if (std::dynamic_pointer_cast<DictionarySegment<T>>(segment)) {
          const auto& column = std::dynamic_pointer_cast<DictionarySegment<T>>(segment);
          const auto& attribute_vector = column->attribute_vector();
          ValueID value_id = ValueID{0};
          bool is_greater_than_whole_chunk = false;
          bool is_unequal_whole_chunk = false;

          value_id = column->lower_bound(type_cast<T>(table_scan.search_value()));

          if (value_id == ValueID{0} && ValueID{0} == column->upper_bound(type_cast<T>(table_scan.search_value()))) {
            if (table_scan.scan_type() == ScanType::OpEquals || table_scan.scan_type() == ScanType::OpLessThanEquals) {
              break;
            }
            if (table_scan.scan_type() == ScanType::OpNotEquals) {
              is_unequal_whole_chunk = true;
            }
            if (table_scan.scan_type() == ScanType::OpGreaterThan) {
              is_greater_than_whole_chunk = true;
            }
          }

          for (ChunkOffset chunk_offset = 0; chunk_offset < attribute_vector->size(); ++chunk_offset) {

            if (is_greater_than_whole_chunk || is_unequal_whole_chunk ||
                compare(attribute_vector->get(chunk_offset), value_id, table_scan.scan_type())) {
              pos_list->emplace_back(RowID({ChunkID{chunk_index}, ChunkOffset{chunk_offset}}));
            }
          }
        } else if (std::dynamic_pointer_cast<ValueSegment<T>>(segment)) {
          const auto& column = std::dynamic_pointer_cast<ValueSegment<T>>(segment);
          const auto& values = column->values();

          for (ChunkOffset chunk_offset = 0; chunk_offset < values.size(); ++chunk_offset) {
            if (compare(values[chunk_offset], type_cast<T>(table_scan.search_value()), table_scan.scan_type())) {
              pos_list->emplace_back(RowID({ChunkID{chunk_index}, ChunkOffset{chunk_offset}}));
            }
          }
        } else if (std::dynamic_pointer_cast<ReferenceSegment>(segment)) {
          const auto& column = std::dynamic_pointer_cast<ReferenceSegment>(segment);
          const auto& row_count = table_scan._input_table_left()->row_count();

          for (ChunkOffset chunk_offset = 0; chunk_offset < row_count; ++chunk_offset) {
            if (compare(type_cast<T>((*column)[chunk_offset]), type_cast<T>(table_scan.search_value()),
                        table_scan.scan_type())) {
              auto const& row_id = (*column->pos_list())[chunk_offset];
              pos_list->emplace_back(RowID({row_id.chunk_id, row_id.chunk_offset}));
            }
          }
        }
      }
      return create_result_table(table_scan, result_size, pos_list);
    }
  };

  std::shared_ptr<const Table> _on_execute() override;

  std::unique_ptr<BaseTableScanImpl> _table_scan_impl;
  const ColumnID _column_id;
  const ScanType _scan_type;
  const AllTypeVariant _search_value;
};

}  // namespace opossum
