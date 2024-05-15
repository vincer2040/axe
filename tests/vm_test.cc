#include "../src/ast.h"
#include "../src/compiler.h"
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/vm.h"
#include <gtest/gtest.h>

axe::ast parse(const std::string& input) {
    axe::lexer l(input);
    axe::parser p(l);
    return p.parse();
}

void test_integer(const axe::object& got, int64_t expected) {
    EXPECT_EQ(got.get_type(), axe::object_type::Integer);
    EXPECT_EQ(got.get_int(), expected);
}

template <typename T> struct vm_test {
    std::string input;
    T expected;
};

void run_vm_int_test(const vm_test<int64_t>& test) {
    auto ast = parse(test.input);
    axe::compiler compiler;
    auto err = compiler.compile(std::move(ast));
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    axe::vm vm(compiler.get_byte_code());
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
    };

    for (auto& test : tests) {
        run_vm_int_test(test);
    }
}
