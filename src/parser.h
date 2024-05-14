#ifndef __AXE_PARSER_H__

#define __AXE_PARSER_H__

#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <vector>

namespace axe {

enum class precedence {
    Lowest = 0,
    Equals = 1,
    LessGreater = 2,
    Sum = 3,
    Product = 4,
    Call = 5,
};

class parser {
  public:
    parser(lexer l);
    ast parse();
    const std::vector<std::string>& get_errors() const;

  private:
    lexer lex;
    token cur_token;
    token peek_token;
    std::vector<std::string> errors;

    ast parse_ast();
    statement parse_statement();

    statement parse_expression_statement();

    expression parse_expression(precedence precedence);
    expression parse_integer();

    void next_token();
    bool peek_token_is(token_type type);

    void unknown_token_error(const token& type);
};

} // namespace axe

#endif // __AXE_PARSER_H__
