#ifndef __AXE_PARSER_H__

#define __AXE_PARSER_H__

#include "ast.h"
#include "lexer.h"
#include "token.h"
#include <vector>

namespace axe {

enum class precedence {
    Lowest = 0,
    Assign = 1,
    Equals = 2,
    LessGreater = 3,
    Sum = 4,
    Product = 5,
    Prefix = 6,
    Call = 7,
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
    statement parse_let_statement();
    statement parse_return_statement();
    statement parse_expression_statement();

    expression parse_expression(precedence precedence);
    expression parse_integer();
    expression parse_float();
    expression parse_bool(bool value);
    expression parse_string();
    expression parse_ident();
    expression parse_prefix(prefix_operator op);
    expression parse_infix(infix_operator op, expression lhs);
    expression parse_assign(expression ident);
    expression parse_group();
    expression parse_if();
    expression parse_match();
    expression parse_function();
    expression parse_call(expression name_expr);

    std::vector<match_branch> parse_match_branches();
    std::optional<match_branch> parse_match_branch();
    match_branch_pattern parse_match_branch_pattern();
    std::optional<match_branch_consequence> parse_match_branch_consequence();

    std::vector<std::string> parse_function_params();

    std::vector<expression> parse_call_args();

    block_statement parse_block();

    void next_token();
    bool peek_token_is(token_type type);
    bool expect_peek(token_type type);

    precedence cur_precedence();
    precedence peek_precedence();

    void unknown_token_error(const token& token);
    void peek_error(const token& token);
};

} // namespace axe

#endif // __AXE_PARSER_H__
