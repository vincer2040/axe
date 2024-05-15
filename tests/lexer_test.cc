#include "../src/lexer.h"
#include "../src/token.h"
#include <gtest/gtest.h>

TEST(Lexer, BasicChars) {
    std::string input = "=+-*/(){}<>!:;,._";
    axe::lexer lexer(input);
    axe::token_type expected[] = {
        axe::token_type::Assign,     axe::token_type::Plus,
        axe::token_type::Minus,      axe::token_type::Asterisk,
        axe::token_type::Slash,      axe::token_type::LParen,
        axe::token_type::RParen,     axe::token_type::LSquirly,
        axe::token_type::RSquirly,   axe::token_type::Lt,
        axe::token_type::Gt,         axe::token_type::Bang,
        axe::token_type::Colon,      axe::token_type::Semicolon,
        axe::token_type::Comma,      axe::token_type::Dot,
        axe::token_type::Underscore, axe::token_type::Eof,
    };

    for (auto& exp : expected) {
        auto tok = lexer.next_token();
        EXPECT_EQ(tok.get_type(), exp);
    }
}

struct key_word_test {
    std::string input;
    axe::token_type expected;
};

TEST(Lexer, KeyWords) {
    key_word_test tests[] = {
        {"let", axe::token_type::Let},       {"fn", axe::token_type::Function},
        {"if", axe::token_type::If},         {"else", axe::token_type::Else},
        {"true", axe::token_type::True},     {"false", axe::token_type::False},
        {"return", axe::token_type::Return}, {"match", axe::token_type::Match},
    };

    for (auto& test : tests) {
        axe::lexer lexer(test.input);
        auto got = lexer.next_token();
        EXPECT_EQ(got.get_type(), test.expected);
        EXPECT_EQ(lexer.next_token().get_type(), axe::token_type::Eof);
    }
}

struct literal_test {
    std::string input;
    std::string expected;
};

TEST(Lexer, Idents) {
    literal_test tests[] = {
        {"foo", "foo"},
        {"foo1", "foo1"},
        {"foo_1", "foo_1"},
    };

    for (auto& test : tests) {
        axe::lexer lexer(test.input);
        auto got = lexer.next_token();
        EXPECT_EQ(got.get_type(), axe::token_type::Ident);
        auto literal = got.get_literal();
        EXPECT_STREQ(literal.c_str(), test.expected.c_str());
    }
}

TEST(Lexer, Integers) {
    literal_test tests[] = {
        {"5", "5"},
        {"10", "10"},
        {"123", "123"},
    };

    for (auto& test : tests) {
        axe::lexer lexer(test.input);
        auto got = lexer.next_token();
        EXPECT_EQ(got.get_type(), axe::token_type::Integer);
        auto literal = got.get_literal();
        EXPECT_STREQ(literal.c_str(), test.expected.c_str());
    }
}

TEST(Lexer, Floats) {
    literal_test tests[] = {
        {"5.5", "5.5"},
        {"10.10", "10.10"},
        {"123.123", "123.123"},
    };

    for (auto& test : tests) {
        axe::lexer lexer(test.input);
        auto got = lexer.next_token();
        EXPECT_EQ(got.get_type(), axe::token_type::Float);
        auto literal = got.get_literal();
        EXPECT_STREQ(literal.c_str(), test.expected.c_str());
    }
}

struct token_test {
    std::string input;
    axe::token_type expected;
};

TEST(Lexer, DoubleCharTokens) {
    token_test tests[] = {
        {"==", axe::token_type::Eq},
        {"!=", axe::token_type::NotEq},
        {"->", axe::token_type::Arrow},
        {"=>", axe::token_type::FatArrow},
    };

    for (auto& test : tests) {
        axe::lexer lexer(test.input);
        auto got = lexer.next_token();
        EXPECT_EQ(got.get_type(), test.expected);
        EXPECT_EQ(lexer.next_token().get_type(), axe::token_type::Eof);
    }
}

TEST(Lexer, Strings) {
    std::string input = "\"foobar\" \"foo bar\"";
    const char* expected[] = {"foobar", "foo bar"};
    axe::lexer l(input);
    for (auto& exp : expected) {
        auto got = l.next_token();
        EXPECT_EQ(got.get_type(), axe::token_type::String);
        auto lit = got.get_literal();
        EXPECT_STREQ(lit.c_str(), exp);
    }
}
