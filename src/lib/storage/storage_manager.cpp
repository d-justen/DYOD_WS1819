#include "storage_manager.hpp"

#include <memory>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "utils/assert.hpp"

namespace opossum {

StorageManager& StorageManager::get() {
  static StorageManager storage_manager;
  return storage_manager;
}

void StorageManager::add_table(const std::string& name, std::shared_ptr<Table> table) { _tables[name] = table; }

void StorageManager::drop_table(const std::string& name) {
  if (!_tables.erase(name)) {
    throw std::runtime_error("Table does not exist");
  }
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const {
  if (!has_table(name)) {
    throw std::runtime_error("Table does not exist");
  }
  return _tables.at(name);
}

bool StorageManager::has_table(const std::string& name) const {
  if (_tables.count(name) == 0) {
    return false;
  }
  return true;
}

std::vector<std::string> StorageManager::table_names() const {
  std::vector<std::string> keys;
  keys.reserve(_tables.size());
  for (const auto& table : _tables) {
    keys.push_back(table.first);
  }
  return keys;
}

void StorageManager::print(std::ostream& out) const {
  for (const auto& table : _tables) {
    out << table.first << ", columns: " << table.second->column_count() << ", rows: " << table.second->row_count()
        << ", chunks: " << table.second->chunk_count() << std::endl;
  }
}

void StorageManager::reset() { _tables.clear(); }

}  // namespace opossum
