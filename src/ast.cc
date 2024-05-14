#include "ast.h"
#include "base.h"
#include <string>

namespace axe {

const char* const infix_operator_strings[] = {
    "Plus", "Minus", "Asterisk", "Slash", "Lt", "Gt", "Eq", "NotEq",
};

const char* const infix_operator_string_reps[] = {
    "+", "-", "*", "/", "<", ">", "==", "!=",
};

infix::infix(infix_operator op, std::unique_ptr<expression> lhs,
             std::unique_ptr<expression> rhs)
    : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

infix_operator infix::get_op() const { return this->op; }

const std::unique_ptr<expression>& infix::get_lhs() const { return this->lhs; }

const std::unique_ptr<expression>& infix::get_rhs() const { return this->rhs; }

std::string infix::string() const {
    std::string res = "(";
    res += this->lhs->string();
    res += " ";
    res += infix_operator_string_reps[(int)this->op];
    res += " ";
    res += this->rhs->string();
    res += ")";
    return res;
}

const char* const prefix_operator_strings[] = {
    "Bang",
    "Minus",
};

const char* const prefix_operator_string_reps[] = {
    "!",
    "-",
};

prefix::prefix(prefix_operator op, std::unique_ptr<expression> rhs)
    : op(op), rhs(std::move(rhs)) {}

prefix_operator prefix::get_op() const { return this->op; }

const std::unique_ptr<expression>& prefix::get_rhs() const { return this->rhs; }

std::string prefix::string() const {
    std::string res = "(";
    res += prefix_operator_string_reps[(int)this->op];
    res += this->rhs->string();
    res += ")";
    return res;
}

expression::expression()
    : type(expression_type::Illegal), data(std::monostate()) {}

expression::expression(expression_type type, expression_data data)
    : type(type), data(std::move(data)) {}

const char* const expression_type_strings[] = {
    "Illegal", "Integer", "Float", "Prefix", "Infix",
};

expression_type expression::get_type() const { return this->type; }

int64_t expression::get_int() const {
    AXE_CHECK(this->type == expression_type::Integer,
              "trying to get Integer from type %s",
              expression_type_strings[(int)this->type]);
    return std::get<int64_t>(this->data);
}

double expression::get_float() const {
    AXE_CHECK(this->type == expression_type::Float,
              "trying to get Float from type %s",
              expression_type_strings[(int)this->type]);
    return std::get<double>(this->data);
}

const std::string& expression::get_ident() const {
    AXE_CHECK(this->type == expression_type::Ident,
              "trying to get Ident from type %s",
              expression_type_strings[(int)this->type]);
    return std::get<std::string>(this->data);
}

const prefix& expression::get_prefix() const {
    AXE_CHECK(this->type == expression_type::Prefix,
              "trying to get Prefix from type %s",
              expression_type_strings[(int)this->type]);
    return std::get<prefix>(this->data);
}

const infix& expression::get_infix() const {
    AXE_CHECK(this->type == expression_type::Infix,
              "trying to get Infix from type %s",
              expression_type_strings[(int)this->type]);
    return std::get<infix>(this->data);
}

std::string expression::string() const {
    switch (this->type) {
    case expression_type::Integer:
        return std::to_string(std::get<int64_t>(this->data));
    case expression_type::Float:
        return std::to_string(std::get<double>(this->data));
    case expression_type::Ident:
        return std::get<std::string>(this->data);
    case expression_type::Prefix:
        return std::get<prefix>(this->data).string();
    case expression_type::Infix:
        return std::get<infix>(this->data).string();
    default:
        break;
    }
    AXE_UNREACHABLE;
}

let_statement::let_statement(std::string name, expression value)
    : name(std::move(name)), value(std::move(value)) {}

const std::string& let_statement::get_name() const { return this->name; }

const expression& let_statement::get_value() const { return this->value; }

std::string let_statement::string() const {
    std::string res = "let ";
    res += this->name;
    res += " = ";
    res += this->value.string();
    res += ';';
    return res;
}

statement::statement()
    : type(statement_type::Illegal), data(std::monostate()) {}

statement::statement(statement_type type, statement_data data)
    : type(type), data(std::move(data)) {}

statement_type statement::get_type() const { return this->type; }

const char* const statement_type_strings[] = {
    "Illegal",
    "LetStatement",
    "ReturnStatement",
    "ExpressionStatement",
};

const let_statement& statement::get_let() const {
    AXE_CHECK(this->type == statement_type::LetStatement,
              "trying to get LetStatement from type %s",
              statement_type_strings[(int)this->type]);
    return std::get<let_statement>(this->data);
}

const return_statement& statement::get_return() const {
    AXE_CHECK(this->type == statement_type::ReturnStatement,
              "trying to get ReturnStatement from type %s",
              statement_type_strings[(int)this->type]);
    return std::get<expression>(this->data);
}

const expression& statement::get_expression() const {
    AXE_CHECK(this->type == statement_type::ExpressionStatement,
              "trying to get ExpressionStatement from type %s",
              statement_type_strings[(int)this->type]);
    return std::get<expression>(this->data);
}

std::string statement::string() const {
    switch (this->type) {
    case statement_type::LetStatement:
        return this->get_let().string();
    case statement_type::ReturnStatement:
        return this->get_return().string();
    case statement_type::ExpressionStatement:
        return this->get_expression().string();
    default:
        break;
    }
    AXE_UNREACHABLE;
}

ast::ast(std::vector<statement> statements)
    : statements(std::move(statements)) {}

const std::vector<statement>& ast::get_statements() const {
    return this->statements;
}

std::string ast::string() const {
    std::string res;
    for (auto& statement : statements) {
        res += statement.string();
    }
    return res;
}

} // namespace axe
