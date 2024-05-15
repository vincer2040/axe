#ifndef __AXE_AST_H__

#define __AXE_AST_H__

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace axe {

class ast_node {
  public:
    virtual std::string string() const = 0;
};

enum class prefix_operator {
    Bang,
    Minus,
};

class prefix : public ast_node {
  public:
    prefix(prefix_operator op, std::unique_ptr<class expression> rhs);

    prefix_operator get_op() const;
    const std::unique_ptr<expression>& get_rhs() const;

    std::string string() const;

  private:
    prefix_operator op;
    std::unique_ptr<class expression> rhs;
};

enum class infix_operator {
    Plus,
    Minus,
    Asterisk,
    Slash,
    Lt,
    Gt,
    Eq,
    NotEq,
};

class infix : public ast_node {
  public:
    infix(infix_operator op, std::unique_ptr<class expression> lhs,
          std::unique_ptr<class expression> rhs);

    infix_operator get_op() const;
    const std::unique_ptr<class expression>& get_lhs() const;
    const std::unique_ptr<class expression>& get_rhs() const;

    std::string string() const;

  private:
    infix_operator op;
    std::unique_ptr<class expression> lhs;
    std::unique_ptr<class expression> rhs;
};

class block_statement : public ast_node {
  public:
    block_statement(std::vector<class statement> block);

    const std::vector<class statement>& get_block() const;

    std::string string() const;

  private:
    std::vector<class statement> block;
};

class if_expression : public ast_node {
  public:
    if_expression(std::unique_ptr<class expression> cond,
                  block_statement consequence,
                  std::optional<block_statement> alternative);

    const std::unique_ptr<class expression>& get_cond() const;
    const block_statement& get_consequence() const;
    const std::optional<block_statement>& get_alternative() const;

    std::string string() const;

  private:
    std::unique_ptr<class expression> cond;
    block_statement consequence;
    std::optional<block_statement> alternative;
};

enum class expression_type {
    Illegal,
    Integer,
    Float,
    Bool,
    Ident,
    Prefix,
    Infix,
    If,
};

using expression_data = std::variant<std::monostate, int64_t, double, bool,
                                     std::string, prefix, infix, if_expression>;

class expression : public ast_node {
  public:
    expression();
    expression(expression_type type, expression_data data);

    expression_type get_type() const;
    int64_t get_int() const;
    double get_float() const;
    bool get_bool() const;
    const std::string& get_ident() const;
    const prefix& get_prefix() const;
    const infix& get_infix() const;
    const if_expression& get_if() const;

    std::string string() const;

  private:
    expression_type type;
    expression_data data;
};

class let_statement : public ast_node {
  public:
    let_statement(std::string name, expression value);

    const std::string& get_name() const;
    const expression& get_value() const;

    std::string string() const;

  private:
    std::string name;
    expression value;
};

using return_statement = expression;

enum class statement_type {
    Illegal,
    LetStatement,
    ReturnStatement,
    ExpressionStatement,
};

using statement_data = std::variant<std::monostate, let_statement, expression>;

class statement : public ast_node {
  public:
    statement();
    statement(statement_type type, statement_data data);

    statement_type get_type() const;
    const let_statement& get_let() const;
    const return_statement& get_return() const;
    const expression& get_expression() const;

    std::string string() const;

  private:
    statement_type type;
    statement_data data;
};

class ast : public ast_node {
  public:
    ast(std::vector<statement> statements);

    const std::vector<statement>& get_statements() const;

    std::string string() const;

  private:
    std::vector<statement> statements;
};

} // namespace axe

#endif // __AXE_AST_H__
