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

void StorageManager::add_table(const std::string& name, std::shared_ptr<Table> table) {
  _tables[name] = table;
  std::string s;
  std::vector<std::string> test = table_names();
  s = accumulate(begin(test), end(test), s);
  std::cout << s << std::endl;
}

void StorageManager::drop_table(const std::string& name) {
  if (has_table(name)) {
    _tables.erase(name);
  } else {
    throw std::runtime_error("Table does not exist");
  }
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const {
  if (has_table(name)) {
    return _tables.find(name)->second;
  } else {
    throw std::runtime_error("Table does not exist");
  }
}

bool StorageManager::has_table(const std::string& name) const {
  if (_tables.find(name) == _tables.end()) {
    return false;
  }
  return true;
}

std::vector<std::string> StorageManager::table_names() const {
  std::vector<std::string> keys;
  for (auto k : _tables) {
    keys.push_back(k.first);
  }
  return keys;
}

void StorageManager::print(std::ostream& out) const {
  // Implementation goes here
}

void StorageManager::reset() { _tables.clear(); }

}  // namespace opossum
