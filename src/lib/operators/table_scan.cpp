#include "table_scan.hpp"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "resolve_type.hpp"
#include "storage/table.hpp"

namespace opossum {

TableScan::TableScan(const std::shared_ptr<const AbstractOperator> in, ColumnID column_id, const ScanType scan_type,
                     const AllTypeVariant search_value)
    : AbstractOperator(in), _column_id(column_id), _scan_type(scan_type), _search_value(search_value) {}

ColumnID TableScan::column_id() const { return _column_id; }
ScanType TableScan::scan_type() const { return _scan_type; }
const AllTypeVariant& TableScan::search_value() const { return _search_value; }

std::shared_ptr<const Table> TableScan::_on_execute() {
  const auto& column_type = _input_table_left()->column_type(_column_id);
  _table_scan_impl = make_unique_by_data_type<TableScan::BaseTableScanImpl, TableScan::TableScanImpl>(column_type);
  const auto& return_table = _table_scan_impl->on_execute(*this);
  return return_table;
}

}  // namespace opossum
