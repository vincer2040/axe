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

block_statement::block_statement(std::vector<statement> block)
    : block(std::move(block)) {}

const std::vector<statement>& block_statement::get_block() const {
    return this->block;
}

std::string block_statement::string() const {
    std::string res;
    for (auto& statement : this->block) {
        res += statement.string();
    }
    return res;
}

if_expression::if_expression(std::unique_ptr<expression> cond,
                             block_statement consequence,
                             std::optional<block_statement> alternative)
    : cond(std::move(cond)), consequence(std::move(consequence)),
      alternative(std::move(alternative)) {}

const std::unique_ptr<expression>& if_expression::get_cond() const {
    return this->cond;
}

const block_statement& if_expression::get_consequence() const {
    return this->consequence;
}

const std::optional<block_statement>& if_expression::get_alternative() const {
    return this->alternative;
}

std::string if_expression::string() const {
    std::string res = "if ";
    res += this->cond->string();
    res += " ";
    res += this->consequence.string();
    if (this->alternative.has_value()) {
        res += "else ";
        res += this->alternative->string();
    }
    return res;
}

match_branch_pattern::match_branch_pattern(match_branch_pattern_type type,
                                           match_branch_pattern_data data)
    : type(type), data(std::move(data)) {}

match_branch_pattern_type match_branch_pattern::get_type() const {
    return this->type;
}

const std::unique_ptr<expression>&
match_branch_pattern::get_expression_pattern() const {
    AXE_CHECK(this->type == match_branch_pattern_type::Expression,
              "tried to get expression pattern from wildcard pattern");
    return std::get<std::unique_ptr<expression>>(this->data);
}

std::string match_branch_pattern::string() const {
    std::string res;
    switch (this->type) {
    case match_branch_pattern_type::Expression:
        res += std::get<std::unique_ptr<expression>>(this->data)->string();
        break;
    case match_branch_pattern_type::Wildcard:
        res += "_";
        break;
    }
    return res;
}

match_branch_consequence::match_branch_consequence(
    match_branch_consequence_type type, match_branch_consequence_data data)
    : type(type), data(std::move(data)) {}

match_branch_consequence_type match_branch_consequence::get_type() const {
    return this->type;
}

const std::unique_ptr<expression>&
match_branch_consequence::get_expression_consequence() const {
    AXE_CHECK(this->type == match_branch_consequence_type::Expression,
              "trying to get expression consequence from block statement "
              "consequence");
    return std::get<std::unique_ptr<expression>>(this->data);
}

const block_statement&
match_branch_consequence::get_block_statement_consequence() const {
    AXE_CHECK(this->type == match_branch_consequence_type::BlockStatement,
              "trying to get block statement consequence from expression "
              "consequence");
    return std::get<block_statement>(this->data);
}

std::string match_branch_consequence::string() const {
    std::string res;
    switch (this->type) {
    case match_branch_consequence_type::Expression:
        res += this->get_expression_consequence()->string();
        break;
    case match_branch_consequence_type::BlockStatement:
        res += this->get_block_statement_consequence().string();
        break;
    }
    return res;
}

match_branch::match_branch(match_branch_pattern pattern,
                           match_branch_consequence consequence)
    : pattern(std::move(pattern)), consequence(std::move(consequence)) {}

const match_branch_pattern& match_branch::get_pattern() const {
    return this->pattern;
}

const match_branch_consequence& match_branch::get_consequence() const {
    return this->consequence;
}

std::string match_branch::string() const {
    std::string res = this->pattern.string();
    res += " => ";
    res += this->consequence.string();
    return res;
}

match::match(std::unique_ptr<expression> pattern,
             std::vector<match_branch> branches)
    : pattern(std::move(pattern)), branches(std::move(branches)) {}

const std::unique_ptr<expression>& match::get_patten() const {
    return this->pattern;
}

const std::vector<match_branch>& match::get_branches() const {
    return this->branches;
}

std::string match::string() const {
    std::string res = "match ";
    res += this->pattern->string();
    res += " {\n";
    for (auto& branch : this->branches) {
        res += branch.string();
        res += ",\n";
    }
    res += "\n}";
    return res;
}

function_expression::function_expression(std::string name,
                                         std::vector<std::string> params,
                                         block_statement body)
    : name(std::move(name)), params(std::move(params)), body(std::move(body)) {}

const std::string& function_expression::get_name() const { return this->name; }

const std::vector<std::string>& function_expression::get_params() const {
    return this->params;
}

const block_statement& function_expression::get_body() const {
    return this->body;
}

std::string function_expression::string() const {
    std::string res = "fn ";
    res += this->name;
    res += "(";
    for (size_t i = 0; i < this->params.size(); ++i) {
        res += this->params[i];
        if (i != this->params.size() - 1) {
            res += ", ";
        }
    }
    res += ")";
    res += " {\n";
    res += this->body.string();
    res += "\n}";
    return res;
}

call::call(std::string name, std::vector<expression> args)
    : name(std::move(name)), args(std::move(args)) {}

const std::string& call::get_name() const { return this->name; }

const std::vector<expression>& call::get_args() const { return this->args; }

std::string call::string() const {
    std::string res;
    res += this->name;
    res += "(";
    for (size_t i = 0; i < this->args.size(); ++i) {
        res += this->args[i].string();
        if (i != this->args.size() - 1) {
            res += ", ";
        }
    }
    res += ")";
    return res;
}

expression::expression()
    : type(expression_type::Illegal), data(std::monostate()) {}

expression::expression(expression_type type, expression_data data)
    : type(type), data(std::move(data)) {}

const char* const expression_type_strings[] = {
    "Illegal", "Integer", "Float", "Bool",     "Prefix",
    "Infix",   "If",      "Match", "Function", "Call",
};

expression_type expression::get_type() const { return this->type; }

const char* expression::type_to_string() const {
    return expression_type_strings[(int)this->type];
}

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

bool expression::get_bool() const {
    AXE_CHECK(this->type == expression_type::Bool,
              "trying to get Bool from type %s",
              expression_type_strings[(int)this->type]);
    return std::get<bool>(this->data);
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

const if_expression& expression::get_if() const {
    AXE_CHECK(this->type == expression_type::If,
              "trying to get If from type %s",
              expression_type_strings[(int)this->type]);
    return std::get<if_expression>(this->data);
}

const match& expression::get_match() const {
    AXE_CHECK(this->type == expression_type::Match,
              "trying to get Match from type %s",
              expression_type_strings[(int)this->type]);
    return std::get<match>(this->data);
}

const function_expression& expression::get_function() const {
    AXE_CHECK(this->type == expression_type::Function,
              "trying to get Function from type %s",
              expression_type_strings[(int)this->type]);
    return std::get<function_expression>(this->data);
}

const call& expression::get_call() const {
    AXE_CHECK(this->type == expression_type::Call,
              "trying to get Call from type %s",
              expression_type_strings[(int)this->type]);
    return std::get<call>(this->data);
}

std::string expression::string() const {
    switch (this->type) {
    case expression_type::Integer:
        return std::to_string(std::get<int64_t>(this->data));
    case expression_type::Float:
        return std::to_string(std::get<double>(this->data));
    case expression_type::Bool:
        return std::get<bool>(this->data) ? "true" : "false";
    case expression_type::Ident:
        return std::get<std::string>(this->data);
    case expression_type::Prefix:
        return std::get<prefix>(this->data).string();
    case expression_type::Infix:
        return std::get<infix>(this->data).string();
    case expression_type::If:
        return std::get<if_expression>(this->data).string();
    case expression_type::Match:
        return std::get<match>(this->data).string();
    case expression_type::Function:
        return std::get<function_expression>(this->data).string();
    case expression_type::Call:
        return std::get<call>(this->data).string();
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
