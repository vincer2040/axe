#include "parser.h"
#include "ast.h"
#include "token.h"
#include <utility>

namespace axe {

static precedence get_precedence(token_type type) {
    switch (type) {
    case token_type::Eq:
    case token_type::NotEq:
        return precedence::Equals;
    case token_type::Lt:
    case token_type::Gt:
        return precedence::LessGreater;
    case token_type::Plus:
    case token_type::Minus:
        return precedence::Sum;
    case token_type::Asterisk:
    case token_type::Slash:
        return precedence::Product;
    default:
        break;
    }
    return precedence::Lowest;
}

parser::parser(lexer l) : lex(l) {
    this->next_token();
    this->next_token();
}

ast parser::parse() { return this->parse_ast(); }

const std::vector<std::string>& parser::get_errors() const {
    return this->errors;
}

ast parser::parse_ast() {
    std::vector<statement> statements;
    while (this->cur_token.get_type() != token_type::Eof) {
        auto statement = this->parse_statement();
        if (statement.get_type() != statement_type::Illegal) {
            statements.push_back(std::move(statement));
        }
        this->next_token();
    }
    return ast(std::move(statements));
}

statement parser::parse_statement() {
    return this->parse_expression_statement();
}

statement parser::parse_expression_statement() {
    expression exp = this->parse_expression(precedence::Lowest);
    if (this->peek_token_is(token_type::Semicolon)) {
        this->next_token();
    }
    return statement(statement_type::ExpressionStatement, std::move(exp));
}

expression parser::parse_expression(precedence precedence) {
    expression expression;
    switch (this->cur_token.get_type()) {
    case token_type::Integer:
        expression = this->parse_integer();
        break;
    case token_type::Float:
        expression = this->parse_float();
        break;
    case token_type::True:
        expression = this->parse_bool(true);
        break;
    case token_type::False:
        expression = this->parse_bool(false);
        break;
    case token_type::Ident:
        expression = this->parse_ident();
        break;
    case token_type::Bang:
        expression = this->parse_prefix(prefix_operator::Bang);
        break;
    case token_type::Minus:
        expression = this->parse_prefix(prefix_operator::Minus);
        break;
    case token_type::LParen:
        expression = this->parse_group();
        break;
    default:
        this->unknown_token_error(this->cur_token);
        break;
    }

    while (!peek_token_is(token_type::Semicolon) &&
           precedence < this->peek_precedence()) {
        switch (this->peek_token.get_type()) {
        case token_type::Plus:
            this->next_token();
            expression =
                this->parse_infix(infix_operator::Plus, std::move(expression));
            break;
        case token_type::Minus:
            this->next_token();
            expression =
                this->parse_infix(infix_operator::Minus, std::move(expression));
            break;
        case token_type::Asterisk:
            this->next_token();
            expression = this->parse_infix(infix_operator::Asterisk,
                                           std::move(expression));
            break;
        case token_type::Slash:
            this->next_token();
            expression =
                this->parse_infix(infix_operator::Slash, std::move(expression));
            break;
        case token_type::Lt:
            this->next_token();
            expression =
                this->parse_infix(infix_operator::Lt, std::move(expression));
            break;
        case token_type::Gt:
            this->next_token();
            expression =
                this->parse_infix(infix_operator::Gt, std::move(expression));
            break;
        case token_type::Eq:
            this->next_token();
            expression =
                this->parse_infix(infix_operator::Eq, std::move(expression));
            break;
        case token_type::NotEq:
            this->next_token();
            expression =
                this->parse_infix(infix_operator::NotEq, std::move(expression));
            break;
        default:
            return expression;
        }
    }

    return expression;
}

expression parser::parse_integer() {
    auto& literal = this->cur_token.get_literal();
    int64_t integer = strtoll(literal.c_str(), NULL, 10);
    return expression(expression_type::Integer, integer);
}

expression parser::parse_float() {
    auto& literal = this->cur_token.get_literal();
    double float_value = stod(literal);
    return expression(expression_type::Float, float_value);
}

expression parser::parse_bool(bool value) {
    return expression(expression_type::Bool, value);
}

expression parser::parse_ident() {
    auto& ident = this->cur_token.get_literal();
    return expression(expression_type::Ident, std::move(ident));
}

expression parser::parse_prefix(prefix_operator op) {
    this->next_token();
    auto rhs = std::make_unique<expression>(
        this->parse_expression(precedence::Prefix));
    prefix prefix(op, std::move(rhs));
    return expression(expression_type::Prefix, std::move(prefix));
}

expression parser::parse_infix(infix_operator op, expression lhs) {
    auto lhs_unique = std::make_unique<expression>(std::move(lhs));
    auto precedence = this->cur_precedence();
    this->next_token();
    auto rhs = std::make_unique<expression>(this->parse_expression(precedence));
    infix infix(op, std::move(lhs_unique), std::move(rhs));
    return expression(expression_type::Infix, std::move(infix));
}

expression parser::parse_group() {
    this->next_token();
    expression res = this->parse_expression(precedence::Lowest);
    if (!this->expect_peek(token_type::RParen)) {
        return expression();
    }
    return res;
}

void parser::next_token() {
    std::swap(this->cur_token, this->peek_token);
    this->peek_token = this->lex.next_token();
}

bool parser::peek_token_is(token_type type) {
    return this->peek_token.get_type() == type;
}

bool parser::expect_peek(token_type type) {
    if (!this->peek_token_is(type)) {
        peek_error(token(type));
        return false;
    }
    this->next_token();
    return true;
}

precedence parser::cur_precedence() {
    return get_precedence(this->cur_token.get_type());
}

precedence parser::peek_precedence() {
    return get_precedence(this->peek_token.get_type());
}

void parser::unknown_token_error(const token& token) {
    std::string err = "unknown token: " + std::string(token.type_to_string());
    this->errors.push_back(err);
}

void parser::peek_error(const token& token) {
    std::string err = "expected peek token to be " +
                      std::string(token.type_to_string()) + ", got " +
                      this->peek_token.type_to_string() + " instead";
    this->errors.push_back(err);
}

} // namespace axe
