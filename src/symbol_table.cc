#include "symbol_table.h"
#include <optional>

namespace axe {

symbol::symbol(std::string name, symbol_scope scope, size_t index)
    : name(name), scope(scope), index(index) {}

bool symbol::operator==(const symbol& other) const {
    if (this->index != other.index) {
        return false;
    }
    if (this->scope != other.scope) {
        return false;
    }
    if (this->name != other.name) {
        return false;
    }
    return true;
}

symbol_table::symbol_table() : num_definitions(0) {}

symbol symbol_table::define(std::string name) {
    symbol symb = {name, symbol_scope::GlobalScope, this->num_definitions};
    auto to_insert = std::pair<std::string, symbol>(name, symb);
    this->store.insert(to_insert);
    this->num_definitions++;
    return symb;
}

std::optional<const symbol>
symbol_table::resolve(const std::string& name) const {
    auto it = this->store.find(name);
    if (it == this->store.end()) {
        return std::nullopt;
    }
    return it->second;
}

} // namespace axe
