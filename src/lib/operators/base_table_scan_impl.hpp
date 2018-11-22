#pragma once

#include <memory>

#include "storage/table.hpp"
#include "table_scan.hpp"

namespace opossum {

class BaseTableScanImpl {
  public:
    virtual ~BaseTableScanImpl() = default;
    virtual std::shared_ptr<const Table> on_execute(const TableScan& table_scan) = 0;
};

}