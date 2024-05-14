#ifndef __AXE_TOKEN_H__

#define __AXE_TOKEN_H__

#include <string>
#include <variant>

namespace axe {

enum class token_type {
    Illegal,
    Eof,

    Assign,
    Plus,
    Minus,
    Asterisk,
    Slash,
    Lt,
    Gt,
    Bang,
    Eq,
    NotEq,

    LParen,
    RParen,
    LSquirly,
    RSquirly,
    Comma,
    Semicolon,
    Colon,
    Dot,

    Arrow,
    FatArrow,

    Let,
    Function,
    If,
    Else,
    True,
    False,
    Return,
    Match,

    Ident,
    Integer,
    Float,
};

class token {
  public:
    token();
    token(token_type type);
    token(token_type, const std::string& literal);
    token(const std::string& literal);

    void set_type(token_type type);
    void set_literal(const std::string& literal);

    token_type get_type() const;
    const std::string& get_literal() const;
    const char* type_to_string() const;
    std::string string() const;

  private:
    token_type type;
    std::variant<std::monostate, std::string> literal;
};

} // namespace axe

#endif /* __AXE_TOKEN_H__ */
