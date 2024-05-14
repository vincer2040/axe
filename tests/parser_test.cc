#include "../src/parser.h"
#include <gtest/gtest.h>

template<typename T> struct parser_test {
    std::string input;
    T expected;
};

TEST(Parser, Integer) {
    parser_test<int64_t> tests[] = {
        {"5;", 5},
        {"10;", 10},
    };

    for (auto& test : tests) {
        axe::lexer l(test.input);
        axe::parser p(l);
        auto ast = p.parse();
        auto& statements = ast.get_statements();
        EXPECT_EQ(statements.size(), 1);
        auto& statement = statements[0];
        EXPECT_EQ(statement.get_type(), axe::statement_type::ExpressionStatement);
        auto& expression = statement.get_expression();
        EXPECT_EQ(expression.get_type(), axe::expression_type::Integer);
        auto value = expression.get_int();
        EXPECT_EQ(value, test.expected);
    }
}
