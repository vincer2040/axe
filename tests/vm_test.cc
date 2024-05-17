#include "../src/ast.h"
#include "../src/compiler.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/vm.h"
#include <gtest/gtest.h>

// NOTE: these tests take a long time
// because it constructs a vector with
// 65536 objects in it on vm construction
// find a way to speed these up.

axe::ast parse(const std::string& input) {
    axe::lexer l(input);
    axe::parser p(l);
    return p.parse();
}

void test_integer(const axe::object& got, int64_t expected) {
    EXPECT_EQ(got.get_type(), axe::object_type::Integer);
    EXPECT_EQ(got.get_int(), expected);
}

void test_string(const axe::object& got, std::string expected) {
    EXPECT_EQ(got.get_type(), axe::object_type::String);
    EXPECT_EQ(got.get_string(), expected);
}

template <typename T> struct vm_test {
    std::string input;
    T expected;
};

void run_vm_int_test(const vm_test<int64_t>& test) {
    auto ast = parse(test.input);
    axe::compiler<std::vector<axe::object>, axe::symbol_table> compiler;
    auto err = compiler.compile(std::move(ast));
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    axe::vm<std::vector<axe::object>> vm(compiler.get_byte_code());
    err = vm.run();
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    auto stack_elem = vm.last_popped_stack_element();
    test_integer(stack_elem, test.expected);
}

TEST(VM, IntegerArithmatic) {
    vm_test<int64_t> tests[] = {
        {"1", 1},
        {"2", 2},
        {"1 + 2", 3},
        {"1 - 2", -1},
        {"1 * 2", 2},
        {"4 / 2", 2},
        {"50 / 2 * 2 + 10 - 5", 55},
        {"5 + 5 + 5 + 5 - 10", 10},
        {"2 * 2 * 2 * 2 * 2", 32},
        {"5 * 2 + 10", 20},
        {"5 + 2 * 10", 25},
        {"5 * (2 + 10)", 60},
        {"-5", -5},
        {"-10", -10},
        {"-50 + 100 + -50", 0},
        {"(5 + 10 * 2 + 15 / 3) * 2 + -10", 50},
    };

    for (auto& test : tests) {
        run_vm_int_test(test);
    }
}

void test_bool(const axe::object& got, bool expected) {
    EXPECT_EQ(got.get_type(), axe::object_type::Bool);
    EXPECT_EQ(got.get_bool(), expected);
}

void run_vm_bool_test(const vm_test<bool>& test) {
    auto ast = parse(test.input);
    axe::compiler<std::vector<axe::object>, axe::symbol_table> compiler;
    auto err = compiler.compile(std::move(ast));
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    axe::vm<std::vector<axe::object>> vm(compiler.get_byte_code());
    err = vm.run();
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    auto stack_elem = vm.last_popped_stack_element();
    test_bool(stack_elem, test.expected);
}

TEST(VM, Booleans) {
    vm_test<bool> tests[] = {
        {"true", true},
        {"false", false},
        {"1 < 2", true},
        {"1 > 2", false},
        {"1 < 1", false},
        {"1 > 1", false},
        {"1 == 1", true},
        {"1 != 1", false},
        {"1 == 2", false},
        {"1 != 2", true},
        {"true == true", true},
        {"false == false", true},
        {"true == false", false},
        {"true != false", true},
        {"false != true", true},
        {"(1 < 2) == true", true},
        {"(1 < 2) == false", false},
        {"(1 > 2) == true", false},
        {"(1 > 2) == false", true},
        {"!true", false},
        {"!false", true},
        {"!5", false},
        {"!!true", true},
        {"!!false", false},
        {"!!5", true},
        {"!(if (false) { 5; })", true},
    };

    for (auto& test : tests) {
        run_vm_bool_test(test);
    }
}

TEST(VM, Conditionals) {
    vm_test<int64_t> tests[] = {
        {"if true { 10 }", 10},
        {"if true { 10 } else { 20 }", 10},
        {"if false { 10 } else { 20 } ", 20},
        {"if 1 { 10 }", 10},
        {"if 1 < 2 { 10 }", 10},
        {"if 1 < 2 { 10 } else { 20 }", 10},
        {"if 1 > 2 { 10 } else { 20 }", 20},
        {"if ((if (false) { 10 })) { 10 } else { 20 }", 20},
    };

    for (auto& test : tests) {
        run_vm_int_test(test);
    }
}

void run_vm_null_test(const std::string& test) {
    auto ast = parse(test);
    axe::compiler<std::vector<axe::object>, axe::symbol_table> compiler;
    auto err = compiler.compile(std::move(ast));
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    axe::vm<std::vector<axe::object>> vm(compiler.get_byte_code());
    err = vm.run();
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    auto stack_elem = vm.last_popped_stack_element();
    EXPECT_EQ(stack_elem.get_type(), axe::object_type::Null);
}

TEST(VM, NullConditions) {
    std::string tests[] = {
        "if 1 > 2 { 10 }",
        "if false { 10 }",
    };

    for (auto& test : tests) {
        run_vm_null_test(test);
    }
}

TEST(VM, GlobalLetStatements) {
    vm_test<int64_t> tests[] = {
        {"let one = 1; one", 1},
        {"let one = 1; let two = 2; one + two", 3},
        {"let one = 1; let two = one + one; one + two", 3},
    };
    for (auto& test : tests) {
        run_vm_int_test(test);
    }
}

void run_vm_string_test(vm_test<std::string> test) {
    auto ast = parse(test.input);
    axe::compiler<std::vector<axe::object>, axe::symbol_table> compiler;
    auto err = compiler.compile(std::move(ast));
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    axe::vm<std::vector<axe::object>> vm(compiler.get_byte_code());
    err = vm.run();
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    auto stack_elem = vm.last_popped_stack_element();
    test_string(stack_elem, test.expected);
}

TEST(VM, Strings) {
    vm_test<std::string> tests[] = {
        {"\"axe\"", "axe"},
        {"\"ax\" + \"e\"", "axe"},
        {"\"axe\" + \"lang\"", "axelang"},
    };

    for (auto& test : tests) {
        run_vm_string_test(test);
    }
}

TEST(VM, FunctionCallsNoArgs) {
    vm_test<int64_t> tests[] = {
        {"fn fivePlusTen() { 5 + 10 }; fivePlusTen()", 15},
        {
            "fn one() { 1; };\
            fn two() { 2; };\
            one() + two()",
            3,
        },
        {
            "fn a() { 1 }\
            fn b() { a() + 1 };\
            fn c() { b() + 1 };\
            c()",
            3,
        },
        {
            "fn earlyExit() { return 99; 100; }; earlyExit()",
            99,
        },
        {
            "fn earlyExit() { return 99; return 100; }; earlyExit()",
            99,
        },
    };

    for (auto& test : tests) {
        run_vm_int_test(test);
    }
}

TEST(VM, FunctionNoReturn) {
    std::string tests[] = {
        "fn noReturn() { }; noReturn()",
        "fn noReturn() { }; fn noReturnTwo() { noReturn(); }; noReturn(); "
        "noReturnTwo()",
    };

    for (auto& test : tests) {
        run_vm_null_test(test);
    }
}

TEST(VM, FirstClassFunctions) {
    vm_test<int64_t> tests[] = {
        {
            "fn returnsOne() { 1 }; fn returnsOneReturner() { returnsOne }; "
            "returnsOneReturner()();",
            1,
        },
    };

    for (auto& test : tests) {
        run_vm_int_test(test);
    }
}
