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
    auto exp_str = axe::instructions_string(concatted);
    auto got_str = axe::instructions_string(got);
    // std::cout << "EXPECTED: " << exp_str << '\n';
    // std::cout << "GOT: " << got_str << '\n';
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
    axe::compiler<axe::constants_owned, axe::symbol_table_owned> compiler;
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
            "-1",
            {axe::object(axe::object_type::Integer, 1)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpMinus, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
    };

    for (auto& test : tests) {
        run_compiler_test(test);
    }
}

TEST(Compiler, Floats) {
    compiler_test tests[] = {
        {"5.5",
         {axe::object(axe::object_type::Float, 5.5)},
         {
             axe::make(axe::op_code::OpConstant, {0}),
             axe::make(axe::op_code::OpPop, {}),
         }},
        {"5.5 + 3.3",
         {axe::object(axe::object_type::Float, 5.5),
          axe::object(axe::object_type::Float, 3.3)},
         {
             axe::make(axe::op_code::OpConstant, {0}),
             axe::make(axe::op_code::OpConstant, {1}),
             axe::make(axe::op_code::OpAdd, {}),
             axe::make(axe::op_code::OpPop, {}),
         }},
    };

    for (auto& test : tests) {
        run_compiler_test(test);
    }
}

TEST(Compiler, Booleans) {
    compiler_test tests[] = {
        {
            "true",
            {},
            {axe::make(axe::op_code::OpTrue, {}),
             axe::make(axe::op_code::OpPop, {})},
        },
        {
            "false",
            {},
            {axe::make(axe::op_code::OpFalse, {}),
             axe::make(axe::op_code::OpPop, {})},
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
        {
            "!true",
            {},
            {
                axe::make(axe::op_code::OpTrue, {}),
                axe::make(axe::op_code::OpBang, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
    };
    for (auto& test : tests) {
        run_compiler_test(test);
    }
}

TEST(Compiler, Conditionals) {
    compiler_test tests[] = {
        {
            "if true { 10 }; 3333",
            {axe::object(axe::object_type::Integer, 10),
             axe::object(axe::object_type::Integer, 3333)},
            {
                axe::make(axe::op_code::OpTrue, {}),
                axe::make(axe::op_code::OpJumpNotTruthy, {10}),
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpJump, {11}),
                axe::make(axe::op_code::OpNull, {}),
                axe::make(axe::op_code::OpPop, {}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "if true { 10 } else { 20 }; 3333;",
            {axe::object(axe::object_type::Integer, 10),
             axe::object(axe::object_type::Integer, 20),
             axe::object(axe::object_type::Integer, 3333)},
            {
                axe::make(axe::op_code::OpTrue, {}),
                axe::make(axe::op_code::OpJumpNotTruthy, {10}),
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpJump, {13}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpPop, {}),
                axe::make(axe::op_code::OpConstant, {2}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
    };

    for (auto& test : tests) {
        run_compiler_test(test);
    }
}

TEST(Compiler, GlobalLetStatements) {
    compiler_test tests[] = {
        {
            "let one = 1; let two = 2;",
            {axe::object(axe::object_type::Integer, 1),
             axe::object(axe::object_type::Integer, 2)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpSetGlobal, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpSetGlobal, {1}),
            },
        },
        {
            "let one = 1; one",
            {axe::object(axe::object_type::Integer, 1)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpSetGlobal, {0}),
                axe::make(axe::op_code::OpGetGlobal, {0}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "let one = 1; let two = one; two;",
            {axe::object(axe::object_type::Integer, 1)},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpSetGlobal, {0}),
                axe::make(axe::op_code::OpGetGlobal, {0}),
                axe::make(axe::op_code::OpSetGlobal, {1}),
                axe::make(axe::op_code::OpGetGlobal, {1}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
    };

    for (auto& test : tests) {
        run_compiler_test(test);
    }
}

TEST(Compiler, Assignments) {
    compiler_test tests[] = {
        {"let foo = 1; foo = 2;",
         {
             axe::object(axe::object_type::Integer, 1),
             axe::object(axe::object_type::Integer, 2),
         },
         {
             axe::make(axe::op_code::OpConstant, {0}),
             axe::make(axe::op_code::OpSetGlobal, {0}),
             axe::make(axe::op_code::OpConstant, {1}),
             axe::make(axe::op_code::OpSetGlobal, {0}),
         }},
        {"let foo = 1; let bar = 2; foo = bar;",
         {
             axe::object(axe::object_type::Integer, 1),
             axe::object(axe::object_type::Integer, 2),
         },
         {
             axe::make(axe::op_code::OpConstant, {0}),
             axe::make(axe::op_code::OpSetGlobal, {0}),
             axe::make(axe::op_code::OpConstant, {1}),
             axe::make(axe::op_code::OpSetGlobal, {1}),
             axe::make(axe::op_code::OpGetGlobal, {1}),
             axe::make(axe::op_code::OpSetGlobal, {0}),
         }},
    };

    for (auto& test : tests) {
        run_compiler_test(test);
    }
}

TEST(Compiler, Strings) {
    compiler_test tests[] = {
        {
            "\"axe\"",
            {axe::object(axe::object_type::String, "axe")},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "\"ax\" + \"e\"",
            {axe::object(axe::object_type::String, "ax"),
             axe::object(axe::object_type::String, "e")},
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpAdd, {}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
    };

    for (auto& test : tests) {
        run_compiler_test(test);
    }
}

TEST(Compiler, Functions) {
    compiler_test tests[] = {
        {
            "fn() { return 5 + 10 }",
            {
                axe::object(axe::object_type::Integer, 5),
                axe::object(axe::object_type::Integer, 10),
                axe::object(axe::object_type::Function,
                            axe::compiled_function(
                                concatinate_instructions({
                                    axe::make(axe::op_code::OpConstant, {0}),
                                    axe::make(axe::op_code::OpConstant, {1}),
                                    axe::make(axe::op_code::OpAdd, {}),
                                    axe::make(axe::op_code::OpReturnValue, {}),
                                }),
                                0, 0)),
            },
            {
                axe::make(axe::op_code::OpConstant, {2}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "fn() { 5 + 10 }",
            {
                axe::object(axe::object_type::Integer, 5),
                axe::object(axe::object_type::Integer, 10),
                axe::object(axe::object_type::Function,
                            axe::compiled_function(
                                concatinate_instructions({
                                    axe::make(axe::op_code::OpConstant, {0}),
                                    axe::make(axe::op_code::OpConstant, {1}),
                                    axe::make(axe::op_code::OpAdd, {}),
                                    axe::make(axe::op_code::OpReturnValue, {}),
                                }),
                                0, 0)),
            },
            {
                axe::make(axe::op_code::OpConstant, {2}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "fn() { 1; 2 }",
            {
                axe::object(axe::object_type::Integer, 1),
                axe::object(axe::object_type::Integer, 2),
                axe::object(axe::object_type::Function,
                            axe::compiled_function(
                                concatinate_instructions({
                                    axe::make(axe::op_code::OpConstant, {0}),
                                    axe::make(axe::op_code::OpPop, {}),
                                    axe::make(axe::op_code::OpConstant, {1}),
                                    axe::make(axe::op_code::OpReturnValue, {}),
                                }),
                                0, 0)),
            },
            {
                axe::make(axe::op_code::OpConstant, {2}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "fn() { }",
            {
                axe::object(axe::object_type::Function,
                            axe::compiled_function(
                                concatinate_instructions({
                                    axe::make(axe::op_code::OpReturn, {}),
                                }),
                                0, 0)),
            },
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
    };

    for (auto& test : tests) {
        run_compiler_test(test);
    }
}

TEST(Compiler, FunctionCalls) {
    compiler_test tests[] = {
        {
            "fn() { 24 }()",
            {
                axe::object(axe::object_type::Integer, 24),
                axe::object(axe::object_type::Function,
                            axe::compiled_function(
                                concatinate_instructions({
                                    axe::make(axe::op_code::OpConstant, {0}),
                                    axe::make(axe::op_code::OpReturnValue, {}),
                                }),
                                0, 0)),
            },
            {
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpCall, {0}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "fn no_arg() { 24 } no_arg()",
            {
                axe::object(axe::object_type::Integer, 24),
                axe::object(axe::object_type::Function,
                            axe::compiled_function(
                                concatinate_instructions({
                                    axe::make(axe::op_code::OpConstant, {0}),
                                    axe::make(axe::op_code::OpReturnValue, {}),
                                }),
                                0, 0)),
            },
            {
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpSetGlobal, {0}),
                axe::make(axe::op_code::OpGetGlobal, {0}),
                axe::make(axe::op_code::OpCall, {0}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "fn oneArg(a) { }; oneArg(24)",
            {
                axe::object(axe::object_type::Function,
                            axe::compiled_function(
                                concatinate_instructions({
                                    axe::make(axe::op_code::OpReturn, {}),
                                }),
                                1, 1)),
                axe::object(axe::object_type::Integer, 24),
            },
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpSetGlobal, {0}),
                axe::make(axe::op_code::OpGetGlobal, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpCall, {1}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {
            "fn manyArgs(a, b, c) { }; manyArgs(24, 25, 26)",
            {
                axe::object(axe::object_type::Function,
                            axe::compiled_function(
                                concatinate_instructions({
                                    axe::make(axe::op_code::OpReturn, {}),
                                }),
                                3, 3)),
                axe::object(axe::object_type::Integer, 24),
                axe::object(axe::object_type::Integer, 25),
                axe::object(axe::object_type::Integer, 26),
            },
            {
                axe::make(axe::op_code::OpConstant, {0}),
                axe::make(axe::op_code::OpSetGlobal, {0}),
                axe::make(axe::op_code::OpGetGlobal, {0}),
                axe::make(axe::op_code::OpConstant, {1}),
                axe::make(axe::op_code::OpConstant, {2}),
                axe::make(axe::op_code::OpConstant, {3}),
                axe::make(axe::op_code::OpCall, {3}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
        {"fn oneArg(a) { a }; oneArg(24)",
         {
             axe::object(axe::object_type::Function,
                         axe::compiled_function(
                             concatinate_instructions({
                                 axe::make(axe::op_code::OpGetLocal, {0}),
                                 axe::make(axe::op_code::OpReturnValue, {}),
                             }),
                             1, 1)),
             axe::object(axe::object_type::Integer, 24),
         },
         {
             axe::make(axe::op_code::OpConstant, {0}),
             axe::make(axe::op_code::OpSetGlobal, {0}),
             axe::make(axe::op_code::OpGetGlobal, {0}),
             axe::make(axe::op_code::OpConstant, {1}),
             axe::make(axe::op_code::OpCall, {1}),
             axe::make(axe::op_code::OpPop, {}),
         }},
        {"fn manyArgs(a, b, c) { a; b; c }; manyArgs(24, 25, 26)",
         {
             axe::object(axe::object_type::Function,
                         axe::compiled_function(
                             concatinate_instructions({
                                 axe::make(axe::op_code::OpGetLocal, {0}),
                                 axe::make(axe::op_code::OpPop, {}),
                                 axe::make(axe::op_code::OpGetLocal, {1}),
                                 axe::make(axe::op_code::OpPop, {}),
                                 axe::make(axe::op_code::OpGetLocal, {2}),
                                 axe::make(axe::op_code::OpReturnValue, {}),
                             }),
                             3, 3)),
             axe::object(axe::object_type::Integer, 24),
             axe::object(axe::object_type::Integer, 25),
             axe::object(axe::object_type::Integer, 26),
         },
         {
             axe::make(axe::op_code::OpConstant, {0}),
             axe::make(axe::op_code::OpSetGlobal, {0}),
             axe::make(axe::op_code::OpGetGlobal, {0}),
             axe::make(axe::op_code::OpConstant, {1}),
             axe::make(axe::op_code::OpConstant, {2}),
             axe::make(axe::op_code::OpConstant, {3}),
             axe::make(axe::op_code::OpCall, {3}),
             axe::make(axe::op_code::OpPop, {}),
         }},
    };

    for (auto& test : tests) {
        run_compiler_test(test);
    }
}

TEST(Compiler, LetStatementScopes) {
    compiler_test tests[] = {
        {"let num = 55; fn() { num }",
         {
             axe::object(axe::object_type::Integer, 55),
             axe::object(axe::object_type::Function,
                         axe::compiled_function(
                             concatinate_instructions({
                                 axe::make(axe::op_code::OpGetGlobal, {0}),
                                 axe::make(axe::op_code::OpReturnValue, {}),
                             }),
                             0, 0)),
         },
         {
             axe::make(axe::op_code::OpConstant, {0}),
             axe::make(axe::op_code::OpSetGlobal, {0}),
             axe::make(axe::op_code::OpConstant, {1}),
             axe::make(axe::op_code::OpPop, {}),
         }},
        {"fn() { let num = 55; num }",
         {
             axe::object(axe::object_type::Integer, 55),
             axe::object(axe::object_type::Function,
                         axe::compiled_function(
                             concatinate_instructions({
                                 axe::make(axe::op_code::OpConstant, {0}),
                                 axe::make(axe::op_code::OpSetLocal, {0}),
                                 axe::make(axe::op_code::OpGetLocal, {0}),
                                 axe::make(axe::op_code::OpReturnValue, {}),
                             }),
                             1, 0)),
         },
         {
             axe::make(axe::op_code::OpConstant, {1}),
             axe::make(axe::op_code::OpPop, {}),
         }},
        {
            "fn() { let a = 55; let b = 77; a + b }",
            {
                axe::object(axe::object_type::Integer, 55),
                axe::object(axe::object_type::Integer, 77),
                axe::object(axe::object_type::Function,
                            axe::compiled_function(
                                concatinate_instructions({
                                    axe::make(axe::op_code::OpConstant, {0}),
                                    axe::make(axe::op_code::OpSetLocal, {0}),
                                    axe::make(axe::op_code::OpConstant, {1}),
                                    axe::make(axe::op_code::OpSetLocal, {1}),
                                    axe::make(axe::op_code::OpGetLocal, {0}),
                                    axe::make(axe::op_code::OpGetLocal, {1}),
                                    axe::make(axe::op_code::OpAdd, {}),
                                    axe::make(axe::op_code::OpReturnValue, {}),
                                }),
                                2, 0)),
            },
            {
                axe::make(axe::op_code::OpConstant, {2}),
                axe::make(axe::op_code::OpPop, {}),
            },
        },
    };

    for (auto& test : tests) {
        run_compiler_test(test);
    }
}
