#include "../src/ast.h"
#include "../src/code.h"
#include "../src/compiler.h"
#include "../src/lexer.h"
#include "../src/object.h"
#include "../src/parser.h"
#include <gtest/gtest.h>

struct compiler_test {
    std::string input;
    std::vector<axe::object> expected_constants;
    std::vector<axe::instructions> expected_instructions;
};

static axe::ast parse(const std::string& input) {
    axe::lexer l(input);
    axe::parser p(l);
    return p.parse();
}

axe::instructions
concatinate_instructions(const std::vector<axe::instructions>& instructions) {
    axe::instructions res;
    for (auto& ins : instructions) {
        res.insert(res.end(), ins.begin(), ins.end());
    }
    return res;
}

static void test_instructions(const std::vector<axe::instructions>& expected,
                              const axe::instructions& got) {
    auto concatted = concatinate_instructions(expected);
    EXPECT_EQ(concatted.size(), got.size());
    for (size_t i = 0; i < concatted.size(); ++i) {
        EXPECT_EQ(concatted[i], got[i]);
    }
}

static void test_constants(const std::vector<axe::object> expected,
                           const std::vector<axe::object> got) {
    EXPECT_EQ(expected.size(), got.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        EXPECT_EQ(expected[i], got[i]);
    }
}

static void run_compiler_test(const compiler_test& test) {
    auto ast = parse(test.input);
    axe::compiler compiler;
    auto err = compiler.compile(ast);
    if (err.has_value()) {
        std::cout << *err << '\n';
    }
    EXPECT_FALSE(err.has_value());
    auto& byte_code = compiler.get_byte_code();
    test_instructions(test.expected_instructions, byte_code.ins);
    test_constants(test.expected_constants, byte_code.constants);
}

TEST(Compiler, IntegerArithmatic) {
    compiler_test tests[] = {
        {
            "1 + 2",
            {axe::object(axe::object_type::Integer, 1),
             axe::object(axe::object_type::Integer, 2)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpAdd, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "1; 2",
            {axe::object(axe::object_type::Integer, 1),
             axe::object(axe::object_type::Integer, 2)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpPop, {}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "1 - 2",
            {axe::object(axe::object_type::Integer, 1),
             axe::object(axe::object_type::Integer, 2)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpSub, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "1 * 2",
            {axe::object(axe::object_type::Integer, 1),
             axe::object(axe::object_type::Integer, 2)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpMul, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "2 / 1",
            {axe::object(axe::object_type::Integer, 2),
             axe::object(axe::object_type::Integer, 1)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpDiv, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "1 > 2",
            {axe::object(axe::object_type::Integer, 1),
                axe::object(axe::object_type::Integer, 2)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpGreaterThan, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "1 < 2",
            {axe::object(axe::object_type::Integer, 2),
                axe::object(axe::object_type::Integer, 1)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpGreaterThan, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "1 == 2",
            {axe::object(axe::object_type::Integer, 1),
                axe::object(axe::object_type::Integer, 2)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpEq, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "1 != 2",
            {axe::object(axe::object_type::Integer, 1),
                axe::object(axe::object_type::Integer, 2)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpNotEq, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "true == false",
            {},
            {
                axe::make(axe::op_code::OpTrue, {}),
                axe::make(axe::op_code::OpFalse, {}),
                axe::make(axe::op_code::OpEq, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "true != false",
            {},
            {
                axe::make(axe::op_code::OpTrue, {}),
                axe::make(axe::op_code::OpFalse, {}),
                axe::make(axe::op_code::OpNotEq, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
    };

    for (auto& test : tests) {
        run_compiler_test(test);
    }
}

TEST(Compiler, Booleans) {
    compiler_test tests[] = {
        {"true",
         {},
         {axe::make(axe::op_code::OpTrue, {}),
          axe::make(axe::op_code::OpPop, {})}},
        {"false",
         {},
         {axe::make(axe::op_code::OpFalse, {}),
          axe::make(axe::op_code::OpPop, {})}},
    };
    for (auto& test : tests) {
        run_compiler_test(test);
    }
}
