#include "../src/symbol_table.h"
#include <gtest/gtest.h>

TEST(SymbolTable, Define) {
    std::unordered_map<std::string, axe::symbol> expected = {
        {"a", {"a", axe::symbol_scope::GlobalScope, 0}},
        {"b", {"b", axe::symbol_scope::GlobalScope, 1}},
    };

    axe::symbol_table global;

    auto a = global.define("a");
    auto a_expected = expected.find("a")->second;
    EXPECT_EQ(a, a_expected);

    auto b = global.define("b");
    auto b_expected = expected.find("b")->second;
    EXPECT_EQ(b, b_expected);
}

TEST(SymbolTable, TestResolveGlobal) {
    axe::symbol_table global;
    global.define("a");
    global.define("b");
    std::unordered_map<std::string, axe::symbol> expected = {
        {"a", {"a", axe::symbol_scope::GlobalScope, 0}},
        {"b", {"b", axe::symbol_scope::GlobalScope, 1}},
    };
    for (auto& it : expected) {
        auto expected = it.second;
        auto found = global.resolve(it.first);
        EXPECT_TRUE(found.has_value());
        EXPECT_EQ(*found, it.second);
    }
}
