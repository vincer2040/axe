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

void test_float(const axe::object& got, double expected) {
    EXPECT_EQ(got.get_type(), axe::object_type::Float);
    EXPECT_EQ(got.get_float(), expected);
}

void run_vm_float_test(const vm_test<double>& test) {
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
    test_float(stack_elem, test.expected);
}

TEST(VM, Floats) {
    vm_test<double> tests[] = {
        {"5.5", 5.5},
        {"-5.5", -5.5},
        {"5.5 + 3.3", 8.8},
        {"5.5 - 3.3", 2.2},
        {"5.5 * 3.3", 18.15},
        {"5.0 / 2.5", 2},
    };

    for (auto& test : tests) {
        run_vm_float_test(test);
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

TEST(VM, Assignment) {
    vm_test<int64_t> tests[] = {
        {"let foo = 1; foo = 2; foo", 2},
        {"let foo = 1; let bar = foo; foo = 2; bar", 1},
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
        // TODO: figure out why this doesn't work when doing fn returnsOne() { 1
        // } return returnsOne doesn't work
        {
            "fn returnsOneReturner() {\
                let returnsOne = fn() { 1 }\
                returnsOne\
            }\
            returnsOneReturner()()",
            1,
        },
    };

    for (auto& test : tests) {
        run_vm_int_test(test);
    }
}

TEST(VM, CallingFunctionsWithBindings) {
    vm_test<int64_t> tests[] = {
        {
            "fn one() { let one = 1; one }; one()",
            1,
        },
        {
            "fn oneAndTwo() { let one = 1; let two = 2; one + two }; "
            "oneAndTwo()",
            3,
        },
        {
            "fn oneAndTwo() { let one = 1; let two = 2; one + two }; fn "
            "threeAndFour() { let three = 3; let four = 4; three + four } "
            "oneAndTwo() + threeAndFour()",
            10,
        },
        {
            "fn firstFooBar() { let foobar = 50; foobar }; fn secondFooBar() { "
            "let foobar = 100; foobar }; firstFooBar() + secondFooBar()",
            150,
        },
        {"let globalSeed = 50;\
                fn minusOne() { let num = 1; globalSeed - num }\
                fn minusTwo() { let num = 2; globalSeed - num }\
                minusOne() + minusTwo()",
         97},
    };

    for (auto& test : tests) {
        run_vm_int_test(test);
    }
}

TEST(VM, CallingFunctionsWithArgumentsAndBindings) {
    vm_test<int64_t> tests[] = {
        {
            "fn identity(a) { a } identity(4)",
            4,
        },
        {
            "fn sum(a, b) { a + b }; sum(1, 2)",
            3,
        },
        {
            "fn sum(a, b) { let c = a + b; c } sum(1, 2)",
            3,
        },
        {
            "fn sum(a, b) { let c = a + b; c }; sum(1, 2) + sum(3, 4)",
            10,
        },
        {
            "fn sum(a, b) { let c = a + b; c }; fn outer() { sum(1, 2) + "
            "sum(3, 4) }; outer()",
            10,
        },
        {
            "let globalNum = 10; fn sum(a, b) { let c = a + b; c + globalNum };\
                fn outer() { sum(1, 2) + sum(3, 4) + globalNum }; outer() + globalNum",
            50,
        },
    };

    for (auto& test : tests) {
        run_vm_int_test(test);
    }
}

void run_vm_error_test(const vm_test<std::string>& test) {
    auto ast = parse(test.input);
    axe::compiler<std::vector<axe::object>, axe::symbol_table> compiler;
    auto err = compiler.compile(std::move(ast));
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    axe::vm<std::vector<axe::object>> vm(compiler.get_byte_code());
    err = vm.run();
    EXPECT_TRUE(err.has_value());
    EXPECT_EQ(*err, test.expected);
}

TEST(VM, CallingFunctionsWithWrongArguments) {
    vm_test<std::string> tests[] = {
        {
            "fn() { 1; }(1)",
            "wrong number of arguments: want 0, got 1",
        },
        {
            "fn(a) { a; }()",
            "wrong number of arguments: want 1, got 0",
        },
        {
            "fn(a, b) { a + b }(1)",
            "wrong number of arguments: want 2, got 1",
        },
    };

    for (auto& test : tests) {
        run_vm_error_test(test);
    }
}
