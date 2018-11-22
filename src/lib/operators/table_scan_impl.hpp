#pragma once

#include "abstract_operator.hpp"
#include "all_type_variant.hpp"
#include "types.hpp"
#include "base_table_scan_impl.hpp"
#include "utils/assert.hpp"

namespace opossum {
  
  template <typename T>
  class TableScanImpl : BaseTableScanImpl {
  
  public:
    ~TableScanImpl();

  protected:
    template <typename T>
    std::shared_ptr<const Table> _on_execute(const TableScan& table_scan) {
      const auto& segment = table_scan._input_table_left()->get_chunk(ChunkID{0}).get_segment(table_scan.column_id());
      if (std::dynamic_pointer_cast<DictionarySegment<T>>(segment))) {

      }
      else if (std::dynamic_pointer_cast<ValueSegment<T>>(segment)) {
        for (const auto& chunk: _input_table_left())
      }
    }

    bool compare (const T& lhs, const T& rhs, ScanType scan_type) {
      switch (scan_type) {
        case ScanType::OpEquals: {
          return lhs == rhs;
          break;
        }
        case ScanType::OpGreaterThan: {
          return lhs > rhs;
          break;
        }
        case ScanType::OpGreaterThanEquals: {
          return lhs >= rhs;
          break;
        }
        case ScanType::OpLessThan: {
          return lhs < rhs;
          break;
        }
        case ScanType::OpLessThanEquals: {
          return lhs <= rhs;
          break;
        }
        case ScanType::OpNotEquals: {
          return lhs != rhs;
          break;
        }
      }
    }

    const ColumnID _column_id;
    const ScanType _scan_type;
    const AllTypeVariant _search_value;
  };
}