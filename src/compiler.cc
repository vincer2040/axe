#include "compiler.h"
#include "ast.h"
#include "code.h"
#include <optional>

namespace axe {

std::optional<std::string> compiler::compile(const ast& ast) {
    return this->compile_statements(ast.get_statements());
}

const byte_code compiler::get_byte_code() const {
    return {this->ins, this->constants};
}

int compiler::emit(op_code op, const std::vector<int> operands) {
    auto instructions = make(op, operands);
    int pos = this->add_instruction(instructions);
    return pos;
}

int compiler::add_instruction(const std::vector<uint8_t> ins) {
    size_t pos_new_instruction = this->ins.size();
    this->ins.insert(this->ins.end(), ins.begin(), ins.end());
    return pos_new_instruction;
}

int compiler::add_constant(object obj) {
    this->constants.push_back(std::move(obj));
    return this->constants.size() - 1;
}

std::optional<std::string>
compiler::compile_statements(const std::vector<statement>& statements) {
    for (auto& statement : statements) {
        auto err = this->compile_statement(statement);
        if (err.has_value()) {
            return err;
        }
    }
    return std::nullopt;
}

std::optional<std::string>
compiler::compile_statement(const statement& statement) {
    auto err = this->compile_expression(statement.get_expression());
    if (err.has_value()) {
        return err;
    }
    this->emit(op_code::OpPop, {});
    return std::nullopt;
}

std::optional<std::string>
compiler::compile_expression(const expression& expression) {
    std::optional<std::string> err = std::nullopt;
    switch (expression.get_type()) {
    case expression_type::Integer:
        err = this->compile_integer(expression.get_int());
        break;
    case expression_type::Bool:
        this->emit(expression.get_bool() ? op_code::OpTrue : op_code::OpFalse,
                   {});
        break;
    case expression_type::Infix:
        err = this->compile_infix(expression.get_infix());
        break;
    default:
        err = "cannot compile " + std::string(expression.get_type_string());
        break;
    }
    return err;
}

std::optional<std::string> compiler::compile_integer(int64_t value) {
    object obj(object_type::Integer, value);
    this->emit(op_code::OpConstant, {this->add_constant(std::move(obj))});
    return std::nullopt;
}

std::optional<std::string> compiler::compile_infix(const infix& infix) {

    if (infix.get_op() == infix_operator::Lt) {
        auto err = this->compile_expression(*infix.get_rhs());
        if (err.has_value()) {
            return err;
        }
        err = this->compile_expression(*infix.get_lhs());
        if (err.has_value()) {
            return err;
        }
        this->emit(op_code::OpGreaterThan, {});
        return std::nullopt;
    }

    auto err = this->compile_expression(*infix.get_lhs());
    if (err.has_value()) {
        return err;
    }
    err = this->compile_expression(*infix.get_rhs());
    if (err.has_value()) {
        return err;
    }

    switch (infix.get_op()) {
    case infix_operator::Plus:
        this->emit(op_code::OpAdd, {});
        break;
    case infix_operator::Minus:
        this->emit(op_code::OpSub, {});
        break;
    case infix_operator::Asterisk:
        this->emit(op_code::OpMul, {});
        break;
    case infix_operator::Slash:
        this->emit(op_code::OpDiv, {});
        break;
    case infix_operator::Gt:
        this->emit(op_code::OpGreaterThan, {});
        break;
    case infix_operator::Eq:
        this->emit(op_code::OpEq, {});
        break;
    case infix_operator::NotEq:
        this->emit(op_code::OpNotEq, {});
        break;
    default: {
        auto err = "unknown operator " +
                   std::string(infix_operator_string(infix.get_op()));
        return err;
    }
    }
    return std::nullopt;
}

} // namespace axe
