#include "../src/symbol_table.h"
#include <gtest/gtest.h>

TEST(SymbolTable, Define) {
    std::unordered_map<std::string, axe::symbol> expected = {
        {"a", {"a", axe::symbol_scope::GlobalScope, 0}},
        {"b", {"b", axe::symbol_scope::GlobalScope, 1}},
        {"c", {"c", axe::symbol_scope::LocalScope, 0}},
        {"d", {"d", axe::symbol_scope::LocalScope, 1}},
        {"e", {"e", axe::symbol_scope::LocalScope, 0}},
        {"f", {"f", axe::symbol_scope::LocalScope, 1}},
    };

    axe::symbol_table global;

    auto a = global.define("a");
    auto a_expected = expected.find("a")->second;
    EXPECT_EQ(a, a_expected);

    auto b = global.define("b");
    auto b_expected = expected.find("b")->second;
    EXPECT_EQ(b, b_expected);

    auto first_local = axe::symbol_table::with_outer(global);

    auto c = first_local.define("c");
    auto c_expected = expected.find("c")->second;
    EXPECT_EQ(c, c_expected);

    auto d = first_local.define("d");
    auto d_expected = expected.find("d")->second;
    EXPECT_EQ(d, d_expected);

    auto second_local = axe::symbol_table::with_outer(first_local);

    auto e = second_local.define("e");
    auto e_expected = expected.find("e")->second;
    EXPECT_EQ(e, e_expected);

    auto f = second_local.define("f");
    auto f_expected = expected.find("f")->second;
    EXPECT_EQ(f, f_expected);
}

TEST(SymbolTable, ResolveGlobal) {
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

TEST(SymbolTable, ResolveLocal) {
    axe::symbol_table global;
    global.define("a");
    global.define("b");
    auto local = axe::symbol_table::with_outer(global);
    local.define("c");
    local.define("d");
    std::unordered_map<std::string, axe::symbol> expected = {
        {"a", {"a", axe::symbol_scope::GlobalScope, 0}},
        {"b", {"b", axe::symbol_scope::GlobalScope, 1}},
        {"c", {"c", axe::symbol_scope::LocalScope, 0}},
        {"d", {"d", axe::symbol_scope::LocalScope, 1}},
    };

    for (auto& it : expected) {
        auto expected = it.second;
        auto found = local.resolve(it.first);
        EXPECT_TRUE(found.has_value());
        EXPECT_EQ(*found, expected);
    }
}

struct resolve_nested_local_test {
    axe::symbol_table& table;
    std::vector<axe::symbol> expected;
};

TEST(SymbolTable, ResolveNestedLocal) {
    axe::symbol_table global;
    global.define("a");
    global.define("b");
    auto first_local = axe::symbol_table::with_outer(global);
    first_local.define("c");
    first_local.define("d");
    auto second_local = axe::symbol_table::with_outer(first_local);
    second_local.define("e");
    second_local.define("f");

    resolve_nested_local_test tests[] = {
        {
            first_local,
            {
                {"a", axe::symbol_scope::GlobalScope, 0},
                {"b", axe::symbol_scope::GlobalScope, 1},
                {"c", axe::symbol_scope::LocalScope, 0},
                {"d", axe::symbol_scope::LocalScope, 1},
            },
        },
        {
            second_local,
            {
                {"a", axe::symbol_scope::GlobalScope, 0},
                {"b", axe::symbol_scope::GlobalScope, 1},
                {"e", axe::symbol_scope::LocalScope, 0},
                {"f", axe::symbol_scope::LocalScope, 1},
            },
        },
    };

    for (auto& test : tests) {
        for (auto& symb : test.expected) {
            auto got = test.table.resolve(symb.name);
            EXPECT_TRUE(got.has_value());
            EXPECT_EQ(*got, symb);
        }
    }
}

TEST(SymbolTable, NoValue) {
    axe::symbol_table global;
    auto got = global.resolve("b");
    EXPECT_FALSE(got.has_value());
    auto local = axe::symbol_table::with_outer(global);
    auto got_local = local.resolve("b");
    EXPECT_FALSE(got_local.has_value());
    auto global_out = local.get_outer();
    auto got_global_2 = global_out.resolve("b");
    EXPECT_FALSE(got_global_2.has_value());
}
