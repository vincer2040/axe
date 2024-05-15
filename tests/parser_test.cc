#include "../src/parser.h"
#include <gtest/gtest.h>

void check_errors(axe::parser& p) {
    for (auto& err : p.get_errors()) {
        std::cout << err << '\n';
    }
    EXPECT_EQ(p.get_errors().size(), 0);
}

void test_integer(const axe::expression& expression, int64_t expected) {
    EXPECT_EQ(expression.get_type(), axe::expression_type::Integer);
    auto value = expression.get_int();
    EXPECT_EQ(value, expected);
}

void test_bool(const axe::expression& expression, bool expected) {
    EXPECT_EQ(expression.get_type(), axe::expression_type::Bool);
    auto value = expression.get_bool();
    EXPECT_EQ(value, expected);
}

template <typename T> struct let_test {
    std::string input;
    std::string name;
    T expected;
};

TEST(Parser, LetStatements) {
    let_test<int64_t> tests[] = {
        {"let foo = 1;", "foo", 1},
        {"let foo1 = 1;", "foo1", 1},
        {"let foo_1 = 1;", "foo_1", 1},
    };
    for (auto& test : tests) {
        axe::lexer lexer(test.input);
        axe::parser parser(lexer);
        auto ast = parser.parse();
        check_errors(parser);
        auto& statements = ast.get_statements();
        EXPECT_EQ(statements.size(), 1);
        auto& statement = statements[0];
        EXPECT_EQ(statement.get_type(), axe::statement_type::LetStatement);
        auto& let = statement.get_let();
        EXPECT_STREQ(let.get_name().c_str(), test.name.c_str());
        test_integer(let.get_value(), test.expected);
    }
}

template <typename T> struct parser_test {
    std::string input;
    T expected;
};

TEST(Parser, ReturnStatements) {
    parser_test<int64_t> tests[] = {
        {"return 5;", 5},
        {"return 10;", 10},
    };

    for (auto& test : tests) {
        axe::lexer l(test.input);
        axe::parser p(l);
        auto ast = p.parse();
        check_errors(p);
        auto& statements = ast.get_statements();
        EXPECT_EQ(statements.size(), 1);
        auto& statement = statements[0];
        EXPECT_EQ(statement.get_type(), axe::statement_type::ReturnStatement);
        auto& ret = statement.get_return();
        test_integer(ret, test.expected);
    }
}

TEST(Parser, Integer) {
    parser_test<int64_t> tests[] = {
        {"5;", 5},
        {"10;", 10},
    };

    for (auto& test : tests) {
        axe::lexer l(test.input);
        axe::parser p(l);
        auto ast = p.parse();
        check_errors(p);
        auto& statements = ast.get_statements();
        EXPECT_EQ(statements.size(), 1);
        auto& statement = statements[0];
        EXPECT_EQ(statement.get_type(),
                  axe::statement_type::ExpressionStatement);
        auto& expression = statement.get_expression();
        test_integer(expression, test.expected);
    }
}

TEST(Parser, Floats) {
    parser_test<double> tests[] = {
        {"5.5;", 5.5},
        {"10.10;", 10.10},
    };

    for (auto& test : tests) {
        axe::lexer l(test.input);
        axe::parser p(l);
        auto ast = p.parse();
        check_errors(p);
        auto& statements = ast.get_statements();
        EXPECT_EQ(statements.size(), 1);
        auto& statement = statements[0];
        EXPECT_EQ(statement.get_type(),
                  axe::statement_type::ExpressionStatement);
        auto& expression = statement.get_expression();
        EXPECT_EQ(expression.get_type(), axe::expression_type::Float);
        auto value = expression.get_float();
        EXPECT_EQ(value, test.expected);
    }
}

void test_ident(const axe::expression& expression,
                const std::string& expected) {
    EXPECT_EQ(expression.get_type(), axe::expression_type::Ident);
    auto& value = expression.get_ident();
    EXPECT_STREQ(value.c_str(), expected.c_str());
}

TEST(Parser, Idents) {
    parser_test<std::string> tests[] = {
        {"foo;", "foo"},
        {"foo1;", "foo1"},
        {"foo_1;", "foo_1"},
    };

    for (auto& test : tests) {
        axe::lexer l(test.input);
        axe::parser p(l);
        auto ast = p.parse();
        check_errors(p);
        auto& statements = ast.get_statements();
        EXPECT_EQ(statements.size(), 1);
        auto& statement = statements[0];
        EXPECT_EQ(statement.get_type(),
                  axe::statement_type::ExpressionStatement);
        auto& expression = statement.get_expression();
        test_ident(expression, test.expected);
    }
}

template <typename T> struct prefix_test {
    std::string input;
    axe::prefix_operator op;
    T expected;
};

TEST(Parser, Prefix) {
    prefix_test<int64_t> tests[] = {
        {"!5;", axe::prefix_operator::Bang, 5},
        {"-15;", axe::prefix_operator::Minus, 15},
    };
    for (auto& test : tests) {
        axe::lexer l(test.input);
        axe::parser p(l);
        auto ast = p.parse();
        check_errors(p);
        auto& statements = ast.get_statements();
        EXPECT_EQ(statements.size(), 1);
        auto& statement = statements[0];
        EXPECT_EQ(statement.get_type(),
                  axe::statement_type::ExpressionStatement);
        auto& expression = statement.get_expression();
        EXPECT_EQ(expression.get_type(), axe::expression_type::Prefix);
        auto& prefix = expression.get_prefix();
        EXPECT_EQ(prefix.get_op(), test.op);
        auto& rhs = prefix.get_rhs();
        test_integer(*rhs, test.expected);
    }
}

template <typename T> struct infix_test {
    std::string input;
    axe::infix_operator op;
    T lhs;
    T rhs;
};

TEST(Parser, Infix) {
    infix_test<int64_t> tests[] = {
        {"5 + 5;", axe::infix_operator::Plus, 5, 5},
        {"5 - 5;", axe::infix_operator::Minus, 5, 5},
        {"5 * 5;", axe::infix_operator::Asterisk, 5, 5},
        {"5 / 5;", axe::infix_operator::Slash, 5, 5},
        {"5 < 5;", axe::infix_operator::Lt, 5, 5},
        {"5 > 5;", axe::infix_operator::Gt, 5, 5},
        {"5 == 5;", axe::infix_operator::Eq, 5, 5},
        {"5 != 5;", axe::infix_operator::NotEq, 5, 5},
    };

    for (auto& test : tests) {
        axe::lexer l(test.input);
        axe::parser p(l);
        auto ast = p.parse();
        check_errors(p);
        auto& statements = ast.get_statements();
        EXPECT_EQ(statements.size(), 1);
        auto& statement = statements[0];
        EXPECT_EQ(statement.get_type(),
                  axe::statement_type::ExpressionStatement);
        auto& expression = statement.get_expression();
        EXPECT_EQ(expression.get_type(), axe::expression_type::Infix);
        auto& infix = expression.get_infix();
        EXPECT_EQ(infix.get_op(), test.op);
        test_integer(*infix.get_lhs(), test.lhs);
        test_integer(*infix.get_rhs(), test.rhs);
    }
}

TEST(Parser, OperatorPrecedence) {
    parser_test<std::string> tests[] = {
        {"-a * b", "((-a) * b)"},
        {"!-a", "(!(-a))"},
        {"a + b + c", "((a + b) + c)"},
        {"a + b - c", "((a + b) - c)"},
        {"a * b * c", "((a * b) * c)"},
        {"a * b / c", "((a * b) / c)"},
        {"a + b / c", "(a + (b / c))"},
        {"a + b * c + d / e - f", "(((a + (b * c)) + (d / e)) - f)"},
        {"3 + 4; -5 * 5", "(3 + 4)((-5) * 5)"},
        {"5 > 4 == 3 < 4", "((5 > 4) == (3 < 4))"},
        {"5 < 4 != 3 > 4", "((5 < 4) != (3 > 4))"},
        {"3 + 4 * 5 == 3 * 1 + 4 * 5",
         "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        {"3 + 4 * 5 == 3 * 1 + 4 * 5",
         "((3 + (4 * 5)) == ((3 * 1) + (4 * 5)))"},
        {"true", "true"},
        {"false", "false"},
        {"3 > 5 == false", "((3 > 5) == false)"},
        {"3 < 5 == true", "((3 < 5) == true)"},
        {"1 + (2 + 3) + 4", "((1 + (2 + 3)) + 4)"},
        {"(5 + 5) * 2", "((5 + 5) * 2)"},
        {"2 / (5 + 5)", "(2 / (5 + 5))"},
        {"-(5 + 5)", "(-(5 + 5))"},
        {"!(true == true)", "(!(true == true))"},
        {"a + add(b * c) + d", "((a + add((b * c))) + d)"},
        {"add(a, b, 1, 2 * 3, 4 + 5, add(6, 7 * 8))",
         "add(a, b, 1, (2 * 3), (4 + 5), add(6, (7 * 8)))"},
        {"add(a + b + c * d / f + g)", "add((((a + b) + ((c * d) / f)) + g))"},
    };

    for (auto& test : tests) {
        axe::lexer l(test.input);
        axe::parser p(l);
        auto ast = p.parse();
        check_errors(p);
        auto ast_string = ast.string();
        EXPECT_STREQ(ast_string.c_str(), test.expected.c_str());
    }
}

TEST(Parser, If) {
    std::string input = "if (x > y) { x }";
    axe::lexer l(input);
    axe::parser p(l);
    auto ast = p.parse();
    check_errors(p);
    auto& statements = ast.get_statements();
    EXPECT_EQ(statements.size(), 1);
    auto& statement = statements[0];
    EXPECT_EQ(statement.get_type(), axe::statement_type::ExpressionStatement);
    auto& expression = statement.get_expression();
    EXPECT_EQ(expression.get_type(), axe::expression_type::If);
    auto& if_exp = expression.get_if();
    auto& cond_exp = if_exp.get_cond();
    EXPECT_EQ(cond_exp->get_type(), axe::expression_type::Infix);
    auto& cond = cond_exp->get_infix();
    EXPECT_EQ(cond.get_op(), axe::infix_operator::Gt);
    test_ident(*cond.get_lhs(), "x");
    test_ident(*cond.get_rhs(), "y");
    auto& consequence = if_exp.get_consequence().get_block();
    EXPECT_EQ(consequence.size(), 1);
    auto& consequence_statement = consequence[0];
    EXPECT_EQ(consequence_statement.get_type(),
              axe::statement_type::ExpressionStatement);
    auto& consequence_exp = consequence_statement.get_expression();
    test_ident(consequence_exp, "x");
    EXPECT_EQ(if_exp.get_alternative(), std::nullopt);
}

TEST(Parser, IfElse) {
    std::string input = "if (x > y) { x } else { y }";
    axe::lexer l(input);
    axe::parser p(l);
    auto ast = p.parse();
    check_errors(p);
    auto& statements = ast.get_statements();
    EXPECT_EQ(statements.size(), 1);
    auto& statement = statements[0];
    EXPECT_EQ(statement.get_type(), axe::statement_type::ExpressionStatement);
    auto& expression = statement.get_expression();
    EXPECT_EQ(expression.get_type(), axe::expression_type::If);
    auto& if_exp = expression.get_if();
    auto& cond_exp = if_exp.get_cond();
    EXPECT_EQ(cond_exp->get_type(), axe::expression_type::Infix);
    auto& cond = cond_exp->get_infix();
    EXPECT_EQ(cond.get_op(), axe::infix_operator::Gt);
    test_ident(*cond.get_lhs(), "x");
    test_ident(*cond.get_rhs(), "y");
    auto& consequence = if_exp.get_consequence().get_block();
    EXPECT_EQ(consequence.size(), 1);
    auto& consequence_statement = consequence[0];
    EXPECT_EQ(consequence_statement.get_type(),
              axe::statement_type::ExpressionStatement);
    auto& consequence_exp = consequence_statement.get_expression();
    test_ident(consequence_exp, "x");
    auto& alternative = if_exp.get_alternative();
    EXPECT_TRUE(alternative.has_value());
    auto& alternative_statement = alternative->get_block()[0];
    EXPECT_EQ(alternative_statement.get_type(),
              axe::statement_type::ExpressionStatement);
    auto& alternative_exp = alternative_statement.get_expression();
    test_ident(alternative_exp, "y");
}

TEST(Parser, IfElseNoParen) {
    std::string input = "if x > y { x } else { y }";
    axe::lexer l(input);
    axe::parser p(l);
    auto ast = p.parse();
    check_errors(p);
    auto& statements = ast.get_statements();
    EXPECT_EQ(statements.size(), 1);
    auto& statement = statements[0];
    EXPECT_EQ(statement.get_type(), axe::statement_type::ExpressionStatement);
    auto& expression = statement.get_expression();
    EXPECT_EQ(expression.get_type(), axe::expression_type::If);
    auto& if_exp = expression.get_if();
    auto& cond_exp = if_exp.get_cond();
    EXPECT_EQ(cond_exp->get_type(), axe::expression_type::Infix);
    auto& cond = cond_exp->get_infix();
    EXPECT_EQ(cond.get_op(), axe::infix_operator::Gt);
    test_ident(*cond.get_lhs(), "x");
    test_ident(*cond.get_rhs(), "y");
    auto& consequence = if_exp.get_consequence().get_block();
    EXPECT_EQ(consequence.size(), 1);
    auto& consequence_statement = consequence[0];
    EXPECT_EQ(consequence_statement.get_type(),
              axe::statement_type::ExpressionStatement);
    auto& consequence_exp = consequence_statement.get_expression();
    test_ident(consequence_exp, "x");
    auto& alternative = if_exp.get_alternative();
    EXPECT_TRUE(alternative.has_value());
    auto& alternative_statement = alternative->get_block()[0];
    EXPECT_EQ(alternative_statement.get_type(),
              axe::statement_type::ExpressionStatement);
    auto& alternative_exp = alternative_statement.get_expression();
    test_ident(alternative_exp, "y");
}

template <typename T> struct match_branch_pattern_test {
    axe::match_branch_pattern_type type;
    T expected;
};

template <typename T> struct match_consequence_test {
    axe::match_branch_consequence_type type;
    T expected;
};

TEST(Parser, Match) {
    std::string input = "\
    match foo {\
        1 => true,\
        2 => false,\
        3 => true,\
        _ => { \
            if foo > 10 {\
                false\
            } else {\
                true\
            }\
        }\
    }";
    axe::lexer l(input);
    axe::parser p(l);
    auto ast = p.parse();
    check_errors(p);
    auto& statements = ast.get_statements();
    EXPECT_EQ(statements.size(), 1);
    auto& statement = statements[0];
    EXPECT_EQ(statement.get_type(), axe::statement_type::ExpressionStatement);
    auto& expression = statement.get_expression();
    EXPECT_EQ(expression.get_type(), axe::expression_type::Match);
    auto& match = expression.get_match();
    test_ident(*match.get_patten(), "foo");
    auto& branches = match.get_branches();
    EXPECT_EQ(branches.size(), 4);
    match_branch_pattern_test<int64_t> match_branch_pattern_tests[] = {
        {axe::match_branch_pattern_type::Expression, 1},
        {axe::match_branch_pattern_type::Expression, 2},
        {axe::match_branch_pattern_type::Expression, 3},
        {axe::match_branch_pattern_type::Wildcard, 0},
    };
    size_t length = sizeof match_branch_pattern_tests /
                    sizeof match_branch_pattern_tests[0];
    for (size_t i = 0; i < length; ++i) {
        auto& test = match_branch_pattern_tests[i];
        auto& pattern = branches[i].get_pattern();
        EXPECT_EQ(pattern.get_type(), test.type);
        if (test.type == axe::match_branch_pattern_type::Expression) {
            test_integer(*pattern.get_expression_pattern(), test.expected);
        }
    }
    match_consequence_test<bool> match_consequence_tests[] = {
        {axe::match_branch_consequence_type::Expression, true},
        {axe::match_branch_consequence_type::Expression, false},
        {axe::match_branch_consequence_type::Expression, true},
        {axe::match_branch_consequence_type::BlockStatement, true},
    };
    length = sizeof match_branch_pattern_tests /
             sizeof match_branch_pattern_tests[0];
    for (size_t i = 0; i < length; ++i) {
        auto& test = match_consequence_tests[i];
        auto& consequence = branches[i].get_consequence();
        EXPECT_EQ(consequence.get_type(), test.type);
        if (test.type == axe::match_branch_consequence_type::Expression) {
            test_bool(*consequence.get_expression_consequence(), test.expected);
        } else {
            auto& block =
                consequence.get_block_statement_consequence().get_block();
            EXPECT_EQ(block.size(), 1);
            auto& block_statement = block[0];
            EXPECT_EQ(block_statement.get_type(),
                      axe::statement_type::ExpressionStatement);
            auto& block_expression = block_statement.get_expression();
            EXPECT_EQ(block_expression.get_type(), axe::expression_type::If);
            auto& if_exp = block_expression.get_if();
            auto& cond = if_exp.get_cond();
            EXPECT_EQ(cond->get_type(), axe::expression_type::Infix);
            auto& infix = cond->get_infix();
            EXPECT_EQ(infix.get_op(), axe::infix_operator::Gt);
            test_ident(*infix.get_lhs(), "foo");
            test_integer(*infix.get_rhs(), 10);
            EXPECT_EQ(if_exp.get_consequence().get_block().size(), 1);
            EXPECT_TRUE(if_exp.get_alternative().has_value());
            EXPECT_EQ(if_exp.get_alternative()->get_block().size(), 1);
        }
    }
}

TEST(Parser, MatchParen) {
    std::string input = "\
    match (foo) {\
        1 => true,\
        2 => false,\
        3 => true,\
        _ => { \
            if foo > 10 {\
                false\
            } else {\
                true\
            }\
        }\
    }";
    axe::lexer l(input);
    axe::parser p(l);
    auto ast = p.parse();
    check_errors(p);
    auto& statements = ast.get_statements();
    EXPECT_EQ(statements.size(), 1);
    auto& statement = statements[0];
    EXPECT_EQ(statement.get_type(), axe::statement_type::ExpressionStatement);
    auto& expression = statement.get_expression();
    EXPECT_EQ(expression.get_type(), axe::expression_type::Match);
    auto& match = expression.get_match();
    test_ident(*match.get_patten(), "foo");
    auto& branches = match.get_branches();
    EXPECT_EQ(branches.size(), 4);
    match_branch_pattern_test<int64_t> match_branch_pattern_tests[] = {
        {axe::match_branch_pattern_type::Expression, 1},
        {axe::match_branch_pattern_type::Expression, 2},
        {axe::match_branch_pattern_type::Expression, 3},
        {axe::match_branch_pattern_type::Wildcard, 0},
    };
    size_t length = sizeof match_branch_pattern_tests /
                    sizeof match_branch_pattern_tests[0];
    for (size_t i = 0; i < length; ++i) {
        auto& test = match_branch_pattern_tests[i];
        auto& pattern = branches[i].get_pattern();
        EXPECT_EQ(pattern.get_type(), test.type);
        if (test.type == axe::match_branch_pattern_type::Expression) {
            test_integer(*pattern.get_expression_pattern(), test.expected);
        }
    }
    match_consequence_test<bool> match_consequence_tests[] = {
        {axe::match_branch_consequence_type::Expression, true},
        {axe::match_branch_consequence_type::Expression, false},
        {axe::match_branch_consequence_type::Expression, true},
        {axe::match_branch_consequence_type::BlockStatement, true},
    };
    length = sizeof match_branch_pattern_tests /
             sizeof match_branch_pattern_tests[0];
    for (size_t i = 0; i < length; ++i) {
        auto& test = match_consequence_tests[i];
        auto& consequence = branches[i].get_consequence();
        EXPECT_EQ(consequence.get_type(), test.type);
        if (test.type == axe::match_branch_consequence_type::Expression) {
            test_bool(*consequence.get_expression_consequence(), test.expected);
        } else {
            auto& block =
                consequence.get_block_statement_consequence().get_block();
            EXPECT_EQ(block.size(), 1);
            auto& block_statement = block[0];
            EXPECT_EQ(block_statement.get_type(),
                      axe::statement_type::ExpressionStatement);
            auto& block_expression = block_statement.get_expression();
            EXPECT_EQ(block_expression.get_type(), axe::expression_type::If);
            auto& if_exp = block_expression.get_if();
            auto& cond = if_exp.get_cond();
            EXPECT_EQ(cond->get_type(), axe::expression_type::Infix);
            auto& infix = cond->get_infix();
            EXPECT_EQ(infix.get_op(), axe::infix_operator::Gt);
            test_ident(*infix.get_lhs(), "foo");
            test_integer(*infix.get_rhs(), 10);
            EXPECT_EQ(if_exp.get_consequence().get_block().size(), 1);
            EXPECT_TRUE(if_exp.get_alternative().has_value());
            EXPECT_EQ(if_exp.get_alternative()->get_block().size(), 1);
        }
    }
}

TEST(Parser, Functions) {
    std::string input = "fn add(a, b) { a + b }";
    axe::lexer lexer(input);
    axe::parser parser(lexer);
    auto ast = parser.parse();
    check_errors(parser);
    auto& statements = ast.get_statements();
    EXPECT_EQ(statements.size(), 1);
    auto& statement = statements[0];
    EXPECT_EQ(statement.get_type(), axe::statement_type::ExpressionStatement);
    auto& expression = statement.get_expression();
    EXPECT_EQ(expression.get_type(), axe::expression_type::Function);
    auto& function = expression.get_function();
    EXPECT_STREQ(function.get_name().c_str(), "add");
    const char* expected_params[] = {"a", "b"};
    size_t length = sizeof expected_params / sizeof expected_params[0];
    auto& params = function.get_params();
    EXPECT_EQ(params.size(), length);
    for (size_t i = 0; i < length; ++i) {
        const char* expected = expected_params[i];
        auto& param = params[i];
        EXPECT_STREQ(param.c_str(), expected);
    }
    auto body_str = function.get_body().string();
    EXPECT_STREQ(body_str.c_str(), "(a + b)");
}

TEST(Parser, Call) {
    std::string input = "add(1, 2)";
    axe::lexer lexer(input);
    axe::parser parser(lexer);
    auto ast = parser.parse();
    check_errors(parser);
    auto& statements = ast.get_statements();
    EXPECT_EQ(statements.size(), 1);
    auto& statement = statements[0];
    EXPECT_EQ(statement.get_type(), axe::statement_type::ExpressionStatement);
    auto& expression = statement.get_expression();
    EXPECT_EQ(expression.get_type(), axe::expression_type::Call);
    auto& call = expression.get_call();
    EXPECT_STREQ(call.get_name().c_str(), "add");
    int64_t expected_args[] = {1, 2};
    size_t length = sizeof expected_args / sizeof expected_args[0];
    auto& args = call.get_args();
    EXPECT_EQ(args.size(), length);
    for (size_t i = 0; i < length; ++i) {
        test_integer(args[i], expected_args[i]);
    }
}
