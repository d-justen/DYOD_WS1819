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
  auto t = _tables.find(name);
  if (t != _tables.end()) {
    _tables.erase(t);
  } else {
    throw std::runtime_error("Table does not exist");
  }
}

std::shared_ptr<Table> StorageManager::get_table(const std::string& name) const {
  auto const t = _tables.find(name);
  if (t != _tables.end()) {
    return t->second;
  } else {
    throw std::runtime_error("Table does not exist");
  }
}

bool StorageManager::has_table(const std::string& name) const {
  if (_tables.count(name) == 0) {
    return false;
  }
  return true;
}

std::vector<std::string> StorageManager::table_names() const {
  std::vector<std::string> keys;
  for (auto const& k : _tables) {
    keys.push_back(k.first);
  }
  return keys;
}

void StorageManager::print(std::ostream& out) const {
  std::string s;
  std::vector<std::string> names = table_names();
  s = std::accumulate(std::next(begin(names)), end(names), names[0],
                      [](std::string a, std::string b) { return a + ", " + b; });
  std::cout << s << std::endl;
}

void StorageManager::reset() { _tables.clear(); }

}  // namespace opossum
