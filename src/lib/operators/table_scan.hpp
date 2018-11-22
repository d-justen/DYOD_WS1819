#pragma once

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "abstract_operator.hpp"
#include "all_type_variant.hpp"
#include "storage/dictionary_segment.hpp"
#include "storage/table.hpp"
#include "types.hpp"
#include "utils/assert.hpp"

namespace opossum {

class Table;

class TableScan : public AbstractOperator {
 public:
  TableScan(const std::shared_ptr<const AbstractOperator> in, ColumnID column_id, const ScanType scan_type,
            const AllTypeVariant search_value);

  ~TableScan();

  ColumnID column_id() const;
  ScanType scan_type() const;
  const AllTypeVariant& search_value() const;

 protected:
 class BaseTableScanImpl {
  public:
    virtual ~BaseTableScanImpl() = default;
    BaseTableScanImpl(BaseTableScanImpl&&) = default;
    BaseTableScanImpl& operator=(BaseTableScanImpl&&) = default;
  protected:
    virtual std::shared_ptr<const Table> on_execute(const TableScan& table_scan) = 0;
  };

  template<typename T>
  class TableScanImpl : public BaseTableScanImpl {
    public:
      TableScanImpl() {}
      ~TableScanImpl();

    protected:
      bool compare(const T& lhs, const T& rhs, ScanType scan_type) {
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
          default: {
            return false;
            break;
          }
        }
      }
      
      std::shared_ptr<const Table> _on_execute(const TableScan& table_scan) override {
        auto segment = table_scan._input_table_left()->get_chunk(ChunkID{0}).get_segment(table_scan.column_id());
        if (std::dynamic_pointer_cast<DictionarySegment<T>>(segment)) {

        }
        else if (std::dynamic_pointer_cast<ValueSegment<T>>(segment)) {
          //for (const auto& chunk: _input_table_left())
        }
        return nullptr;
      }
  };
  
  std::shared_ptr<const Table> _on_execute() override;

  const ColumnID _column_id;
  const ScanType _scan_type;
  const AllTypeVariant _search_value;
};

}  // namespace opossum
