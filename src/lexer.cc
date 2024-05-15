#include "lexer.h"
#include <cctype>

namespace axe {

static bool is_valid_start_of_ident(char ch) {
    return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_';
}

static bool is_valid_ident_char(char ch) {
    return 'a' <= ch && ch <= 'z' || 'A' <= ch && ch <= 'Z' || ch == '_' ||
           isdigit(ch);
}

lexer::lexer(const std::string& input) : input(input), position(0), ch(0) {
    this->read_char();
}

token lexer::next_token() {
    token tok;
    this->skip_whitespace();
    switch (this->ch) {
    case 0:
        tok.set_type(token_type::Eof);
        break;
    case '=': {
        char peek = this->peek_char();
        if (peek == '>') {
            this->read_char();
            tok.set_type(token_type::FatArrow);
        } else if (peek == '=') {
            this->read_char();
            tok.set_type(token_type::Eq);
        } else {
            tok.set_type(token_type::Assign);
        }
    } break;
    case '+':
        tok.set_type(token_type::Plus);
        break;
    case '-':
        if (this->peek_char() == '>') {
            this->read_char();
            tok.set_type(token_type::Arrow);
        } else {
            tok.set_type(token_type::Minus);
        }
        break;
    case '*':
        tok.set_type(token_type::Asterisk);
        break;
    case '/':
        tok.set_type(token_type::Slash);
        break;
    case '<':
        tok.set_type(token_type::Lt);
        break;
    case '>':
        tok.set_type(token_type::Gt);
        break;
    case '!':
        if (this->peek_char() == '=') {
            this->read_char();
            tok.set_type(token_type::NotEq);
        } else {
            tok.set_type(token_type::Bang);
        }
        break;
    case '(':
        tok.set_type(token_type::LParen);
        break;
    case ')':
        tok.set_type(token_type::RParen);
        break;
    case '{':
        tok.set_type(token_type::LSquirly);
        break;
    case '}':
        tok.set_type(token_type::RSquirly);
        break;
    case ',':
        tok.set_type(token_type::Comma);
        break;
    case ';':
        tok.set_type(token_type::Semicolon);
        break;
    case ':':
        tok.set_type(token_type::Colon);
        break;
    case '.':
        tok.set_type(token_type::Dot);
        break;
    case '_':
        tok.set_type(token_type::Underscore);
        break;
    default:
        if (is_valid_start_of_ident(this->ch)) {
            std::string ident = this->read_ident();
            return token(ident);
        } else if (isdigit(this->ch)) {
            std::string literal = this->read_integer();
            if (this->ch == '.' && isdigit(this->peek_char())) {
                this->read_char();
                std::string decimal = this->read_integer();
                return token(token_type::Float, literal + "." + decimal);
            } else {
                return token(token_type::Integer, literal);
            }
        }
        break;
    }
    this->read_char();
    return tok;
}

char lexer::peek_char() {
    if (this->position >= this->input.size()) {
        return 0;
    }
    return this->input[this->position];
}

void lexer::read_char() {
    if (this->position >= this->input.size()) {
        this->ch = 0;
        return;
    }
    this->ch = this->input[this->position];
    this->position++;
}

std::string lexer::read_ident() {
    std::string res;
    while (is_valid_ident_char(this->ch)) {
        res.push_back(this->ch);
        this->read_char();
    }
    return res;
}

std::string lexer::read_integer() {
    std::string res;
    while (isdigit(this->ch)) {
        res.push_back(this->ch);
        this->read_char();
    }
    return res;
}

void lexer::skip_whitespace() {
    while (this->ch == ' ' || this->ch == '\t' || this->ch == '\r' ||
           this->ch == '\n') {
        this->read_char();
    }
}

} // namespace axe
