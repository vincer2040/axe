#ifndef __SYMBOL_TABLE_H__

#define __SYMBOL_TABLE_H__

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>

namespace axe {

enum class symbol_scope {
    GlobalScope,
    LocalScope,
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
    static symbol_table with_outer(symbol_table& outer);
    symbol define(std::string name);
    std::optional<const symbol> resolve(const std::string& name) const;
    void erase(const std::string& name);
    symbol_table get_outer();
    size_t get_num_definitions() const;

  private:
    std::unordered_map<std::string, symbol> store;
    std::optional<std::shared_ptr<symbol_table>> outer;
    size_t num_definitions;
};

} // namespace axe

#endif // __SYMBOL_TABLE_H__
