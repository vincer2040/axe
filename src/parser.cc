#include "parser.h"
#include <utility>

namespace axe {

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
    return ast(statements);
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
    default:
        this->unknown_token_error(this->cur_token);
        break;
    }
    return expression;
}

expression parser::parse_integer() {
    auto& literal = this->cur_token.get_literal();
    int64_t integer = strtoll(literal.c_str(), NULL, 10);
    return expression(expression_type::Integer, integer);
}

void parser::next_token() {
    std::swap(this->cur_token, this->peek_token);
    this->peek_token = this->lex.next_token();
}

bool parser::peek_token_is(token_type type) {
    return this->peek_token.get_type() == type;
}

void parser::unknown_token_error(const token& token) {
    std::string err = "unknown token: " + std::string(token.type_to_string());
    this->errors.push_back(err);
}

} // namespace axe
