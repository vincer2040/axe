#ifndef __SYMBOL_TABLE_H__

#define __SYMBOL_TABLE_H__

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>

namespace axe {

enum class symbol_scope {
    GlobalScope,
};

struct symbol {
    std::string name;
    symbol_scope scope;
    size_t index;

    symbol(std::string name, symbol_scope scope, size_t index);
    bool operator==(const symbol& other) const;
};

class symbol_table {
  public:
    symbol_table();
    symbol define(std::string name);
    std::optional<const symbol> resolve(const std::string& name) const;

  private:
    std::unordered_map<std::string, symbol> store;
    size_t num_definitions;
};

} // namespace axe

#endif // __SYMBOL_TABLE_H__
