#include "symbol_table.h"
#include "base.h"
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

symbol_table::symbol_table() : outer(std::nullopt), num_definitions(0) {}

symbol_table symbol_table::with_outer(symbol_table& outer) {
    symbol_table res;
    res.outer = std::make_shared<symbol_table>(outer);
    return res;
}

symbol symbol_table::define(std::string name) {
    symbol symb = {name, symbol_scope::GlobalScope, this->num_definitions};
    if (this->outer.has_value()) {
        symb.scope = symbol_scope::LocalScope;
    }
    auto to_insert = std::pair<std::string, symbol>(name, symb);
    this->store.insert(to_insert);
    this->num_definitions++;
    return symb;
}

std::optional<const symbol>
symbol_table::resolve(const std::string& name) const {
    auto it = this->store.find(name);
    if (it != this->store.end()) {
        return it->second;
    }
    if (this->outer.has_value()) {
        return (*this->outer)->resolve(name);
    }
    return std::nullopt;
}

symbol_table symbol_table::get_outer() {
    AXE_CHECK(this->outer.has_value(),
              "trying to get outer from symbol table without outer");
    return **this->outer;
}

size_t symbol_table::get_num_definitions() const {
    return this->num_definitions;
}

} // namespace axe
