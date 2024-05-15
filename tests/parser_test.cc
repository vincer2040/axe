#include "../src/parser.h"
#include <gtest/gtest.h>

void check_errors(axe::parser& p) {
    for (auto& err : p.get_errors()) {
        std::cout << err << '\n';
    }
    EXPECT_EQ(p.get_errors().size(), 0);
}

template <typename T> struct parser_test {
    std::string input;
    T expected;
};

void test_integer(const axe::expression& expression, int64_t expected) {
    EXPECT_EQ(expression.get_type(), axe::expression_type::Integer);
    auto value = expression.get_int();
    EXPECT_EQ(value, expected);
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
