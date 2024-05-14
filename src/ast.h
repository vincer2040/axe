#ifndef __AXE_AST_H__

#define __AXE_AST_H__

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace axe {

class ast_node {
  public:
    virtual std::string string() const = 0;
};

enum class expression_type {
    Illegal,
    Integer,
    Float,
    Ident,
};

using expression_data =
    std::variant<std::monostate, int64_t, double, std::string>;

class expression : public ast_node {
  public:
    expression();
    expression(expression_type type, expression_data data);

    expression_type get_type() const;
    int64_t get_int() const;
    double get_float() const;
    const std::string& get_ident() const;
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
