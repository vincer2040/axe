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
    return this->compile_expression(statement.get_expression());
}

std::optional<std::string>
compiler::compile_expression(const expression& expression) {
    switch (expression.get_type()) {
    case expression_type::Integer:
        return this->compile_integer(expression.get_int());
    case expression_type::Infix:
        return this->compile_infix(expression.get_infix());
    default:
        break;
    }
    std::string err =
        "cannot compile " + std::string(expression.get_type_string());
    return err;
}

std::optional<std::string> compiler::compile_integer(int64_t value) {
    object obj(object_type::Integer, value);
    this->emit(op_code::OpConstant, {this->add_constant(std::move(obj))});
    return std::nullopt;
}

std::optional<std::string> compiler::compile_infix(const infix& infix) {
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
    default: {
        auto err = "unknown operator " + std::to_string((int)infix.get_op());
        return err;
    }
    }
    return std::nullopt;
}

} // namespace axe