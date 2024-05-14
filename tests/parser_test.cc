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
        EXPECT_EQ(expression.get_type(), axe::expression_type::Ident);
        auto& value = expression.get_ident();
        EXPECT_EQ(value, test.expected);
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
