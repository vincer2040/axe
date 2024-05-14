#include "token.h"
#include "base.h"
#include <memory.h>

namespace axe {

token::token() : type(token_type::Illegal), literal(std::monostate()) {}

token::token(token_type type) : type(type), literal(std::monostate()) {}

token::token(token_type type, const std::string& literal)
    : type(type), literal(literal) {}

struct token_lookup {
    const char* str;
    size_t str_len;
    token_type type;
};

static const token_lookup token_lookups[] = {
    {"let", 3, token_type::Let},     {"fn", 2, token_type::Function},
    {"if", 2, token_type::If},       {"else", 4, token_type::Else},
    {"true", 4, token_type::True},   {"false", 5, token_type::False},
    {"match", 5, token_type::Match}, {"return", 6, token_type::Return},
};

static const size_t token_lookups_size =
    sizeof token_lookups / sizeof token_lookups[0];

token::token(const std::string& literal)
    : type(token_type::Illegal), literal(std::monostate()) {

    size_t literal_length = literal.size();
    const char* literal_cstr = literal.c_str();

    for (size_t i = 0; i < token_lookups_size; ++i) {
        token_lookup lookup = token_lookups[i];
        if (lookup.str_len != literal_length) {
            continue;
        }
        if (memcmp(lookup.str, literal_cstr, lookup.str_len) != 0) {
            continue;
        }
        this->type = lookup.type;
        return;
    }

    this->type = token_type::Ident;
    this->literal = literal;
}

void token::set_type(token_type type) { this->type = type; }

void token::set_literal(const std::string& literal) { this->literal = literal; }

const char* const token_type_strings[] = {
    "Illegal",   "Eof",

    "Assign",    "Plus",     "Minus",    "Asterisk", "Slash",
    "Lt",        "Gt",       "Bang",     "Eq",       "NotEq",

    "LParen",    "RParen",   "LSquirly", "RSquirly", "Comma",
    "Semicolon", "Colon",    "Dot",

    "Arrow",     "FatArrow",

    "Let",       "Function", "If",       "Else",     "True",
    "False",     "Return",   "Match",

    "Ident",     "Integer",  "Float",
};

token_type token::get_type() const { return this->type; }

const char* token::type_to_string() const {
    return token_type_strings[(int)this->type];
}

const std::string& token::get_literal() const {
    AXE_CHECK(this->type == token_type::Integer ||
                  this->type == token_type::Ident ||
                  this->type == token_type::Float,
              "tried to get literal from type %s", this->type_to_string());
    return std::get<std::string>(this->literal);
}

std::string token::string() const {
    std::string res(this->type_to_string());
    if (this->type == token_type::Ident || this->type == token_type::Integer ||
        this->type == token_type::Float) {
        res.push_back(' ');
        res += std::get<std::string>(this->literal);
    }
    return res;
}

} // namespace axe
