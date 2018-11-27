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

    virtual std::shared_ptr<const Table> on_execute() = 0;
  };

  template <typename T>
  class TableScanImpl : public BaseTableScanImpl {
   public:
    TableScanImpl(const std::shared_ptr<const Table> table, ColumnID column_id, const ScanType scan_type,
                  const AllTypeVariant search_value)
        : _table(table), _column_id(column_id), _scan_type(scan_type), _search_value(search_value){}
    ~TableScanImpl() = default;

   protected:
    std::shared_ptr<const Table> _table;
    const ColumnID _column_id;
    const ScanType _scan_type;
    const AllTypeVariant _search_value;

    template <typename U>
    std::function<bool(const U&, const U&)> compare() {
      switch (_scan_type) {
        case ScanType::OpEquals:
          return [](const U& lhs, const U& rhs) { return lhs == rhs; };
          break;
        case ScanType::OpGreaterThan:
          return [](const U& lhs, const U& rhs) { return lhs > rhs; };
          break;
        case ScanType::OpGreaterThanEquals:
          return [](const U& lhs, const U& rhs) { return lhs >= rhs; };
          break;
        case ScanType::OpLessThan:
          return [](const U& lhs, const U& rhs) { return lhs < rhs; };
          break;
        case ScanType::OpLessThanEquals:
          return [](const U& lhs, const U& rhs) { return lhs <= rhs; };
          break;
        case ScanType::OpNotEquals:
          return [](const U& lhs, const U& rhs) { return lhs != rhs; };
          break;
        default:
          return [](const U& lhs, const U& rhs) { return false; };
          break;
      }
    }

    // Iterate over all columns of the input table and create reference segments using the pos list.
    // If the column is a reference segment, use the referenced table to create the new reference segment.
    // Add the segments and their definitions to a single chunk of a new table.
    std::shared_ptr<Table> create_result_table(uint64_t result_size, const std::shared_ptr<PosList> pos_list) {
      const auto& result_table = std::make_shared<Table>(result_size);

      for (ColumnID segment_index = ColumnID{0}; segment_index < _table->column_count(); ++segment_index) {
        std::shared_ptr<ReferenceSegment> reference_segment;

        if (const auto& column =
                std::dynamic_pointer_cast<ReferenceSegment>(_table->get_chunk(ChunkID{0}).get_segment(segment_index))) {
          reference_segment = std::make_shared<ReferenceSegment>(column->referenced_table(), segment_index, pos_list);
        } else {
          reference_segment = std::make_shared<ReferenceSegment>(_table, segment_index, pos_list);
        }
        result_table->get_chunk(ChunkID{0}).add_segment(reference_segment);
        result_table->add_column_definition(_table->column_name(segment_index), _table->column_type(segment_index));
      }
      return result_table;
    }

    std::shared_ptr<const Table> on_execute() {
      const auto& pos_list = std::make_shared<PosList>();
      uint64_t result_size = 0;

      DebugAssert(_search_value.type() == typeid(T), "Types cannot be compared");

      // Iterate over all chunks of the input table.
      for (ChunkID chunk_index = ChunkID{0}; chunk_index < _table->chunk_count(); ++chunk_index) {
        const auto& segment = _table->get_chunk(chunk_index).get_segment(_column_id);

        // Determine if the search column in the chunk is a dictionary segment.
        if (const auto& column = std::dynamic_pointer_cast<DictionarySegment<T>>(segment)) {
          const auto& attribute_vector = column->attribute_vector();
          ValueID value_id = ValueID{0};
          bool is_greater_than_whole_chunk = false;
          bool is_unequal_whole_chunk = false;

          // Create the compare function for the respective scan type.
          const auto& compare_function = compare<ValueID>();

          // Get the value id of a value not less than the search value in the dictionary.
          value_id = column->lower_bound(type_cast<T>(_search_value));

          // For ScanType::OpEquals, ScanType::OpLessThanEquals, ScanType::OpNotEquals and ScanType::OpGreaterThan
          // it can happen that the values in the whole segment are larger than the search value and lower_bound returns
          // the value id 0 even though the corresponding attribute vector entry points to another value in the dictionary.
          // To determine that we do not need to check the segment, the upper_bound is checked. If it does not return a
          // value greater than 0, the value is definitely smaller than the search value and for ScanType::OpEquals and
          // ScanType::OpLessThanEquals we can check the next chunk. For ScanType::OpNotEquals and ScanType::OpGreaterThan
          // we can add the whole chunk to the result.
          if (value_id == ValueID{0} && ValueID{0} == column->upper_bound(type_cast<T>(_search_value))) {
            if (_scan_type == ScanType::OpEquals || _scan_type == ScanType::OpLessThanEquals) {
              continue;
            }
            if (_scan_type == ScanType::OpNotEquals) {
              is_unequal_whole_chunk = true;
            }
            if (_scan_type == ScanType::OpGreaterThan) {
              is_greater_than_whole_chunk = true;
            }
          }

          // Add entry to pos list using the compare function.
          for (ChunkOffset chunk_offset = 0; chunk_offset < attribute_vector->size(); ++chunk_offset) {
            if (is_greater_than_whole_chunk || is_unequal_whole_chunk ||
                compare_function(attribute_vector->get(chunk_offset), value_id)) {
              pos_list->emplace_back(RowID({ChunkID{chunk_index}, ChunkOffset{chunk_offset}}));
            }
          }

          // Determine if the search column in the chunk is a value segment.
        } else if (const auto& column = std::dynamic_pointer_cast<ValueSegment<T>>(segment)) {
          // Create the compare function for the respective scan type.
          const auto& compare_function = compare<T>();
          const auto& values = column->values();

          // Add entry to pos list using the compare function.
          for (ChunkOffset chunk_offset = 0; chunk_offset < values.size(); ++chunk_offset) {
            if (compare_function(values[chunk_offset], type_cast<T>(_search_value))) {
              pos_list->emplace_back(RowID({ChunkID{chunk_index}, ChunkOffset{chunk_offset}}));
            }
          }

          // Determine if the search column in the chunk is a reference segment.
        } else if (const auto& column = std::dynamic_pointer_cast<ReferenceSegment>(segment)) {
          // Create the compare function for the respective scan type.
          const auto& compare_function = compare<T>();
          const auto& row_count = _table->row_count();

          // Add entry to pos list using the compare function.
          for (ChunkOffset chunk_offset = 0; chunk_offset < row_count; ++chunk_offset) {
            if (compare_function(type_cast<T>((*column)[chunk_offset]), type_cast<T>(_search_value))) {
              auto const& row_id = (*column->pos_list())[chunk_offset];
              pos_list->emplace_back(RowID({row_id.chunk_id, row_id.chunk_offset}));
            }
          }
        }
      }
      return create_result_table(result_size, pos_list);
    }
  };

  std::shared_ptr<const Table> _on_execute() override;

  std::unique_ptr<BaseTableScanImpl> _table_scan_impl;
  const ColumnID _column_id;
  const ScanType _scan_type;
  const AllTypeVariant _search_value;
};

}  // namespace opossum
