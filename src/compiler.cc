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

size_t compiler::emit(op_code op, const std::vector<int> operands) {
    auto instructions = make(op, operands);
    int pos = this->add_instruction(instructions);
    this->set_last_instruction(op, pos);
    return pos;
}

size_t compiler::add_instruction(const std::vector<uint8_t> ins) {
    size_t pos_new_instruction = this->ins.size();
    this->ins.insert(this->ins.end(), ins.begin(), ins.end());
    return pos_new_instruction;
}

int compiler::add_constant(object obj) {
    this->constants.push_back(std::move(obj));
    return this->constants.size() - 1;
}

void compiler::set_last_instruction(op_code op, size_t position) {
    emitted_instruction last = {op, position};
    emitted_instruction previous = this->last_instruction;
    this->previous_instruction = previous;
    this->last_instruction = last;
}

bool compiler::last_instruction_is_pop() {
    return this->last_instruction.op == op_code::OpPop;
}

void compiler::remove_last_pop() {
    this->ins.pop_back();
    this->last_instruction = this->previous_instruction;
}

void compiler::replace_instruction(size_t position,
                                   std::vector<uint8_t> new_instruction) {
    for (size_t i = 0; i < new_instruction.size(); ++i) {
        this->ins[position + i] = new_instruction[i];
    }
}

void compiler::change_operand(size_t op_position, int operand) {
    op_code op = static_cast<op_code>(this->ins[op_position]);
    auto new_instruction = make(op, {operand});
    this->replace_instruction(op_position, new_instruction);
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
    case expression_type::Prefix:
        err = this->compile_prefix(expression.get_prefix());
        break;
    case expression_type::Infix:
        err = this->compile_infix(expression.get_infix());
        break;
    case expression_type::If:
        err = this->compile_if(expression.get_if());
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

std::optional<std::string> compiler::compile_prefix(const prefix& prefix) {
    auto err = this->compile_expression(*prefix.get_rhs());
    if (err.has_value()) {
        return err;
    }
    switch (prefix.get_op()) {
    case prefix_operator::Bang:
        this->emit(op_code::OpBang, {});
        break;
    case prefix_operator::Minus:
        this->emit(op_code::OpMinus, {});
        break;
    }
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

std::optional<std::string> compiler::compile_if(const if_expression& if_exp) {
    auto err = this->compile_expression(*if_exp.get_cond());
    if (err.has_value()) {
        return err;
    }

    size_t jump_not_truthy_position =
        this->emit(op_code::OpJumpNotTruthy, {9999});

    err = this->compile_block(if_exp.get_consequence());
    if (err.has_value()) {
        return err;
    }
    if (this->last_instruction_is_pop()) {
        this->remove_last_pop();
    }

    size_t jump_position = this->emit(op_code::OpJump, {9999});
    size_t after_consequence_position = this->ins.size();
    this->change_operand(jump_not_truthy_position, after_consequence_position);

    auto& alternative = if_exp.get_alternative();
    if (!alternative.has_value()) {
        this->emit(op_code::OpNull, {});
    } else {
        err = this->compile_block(*alternative);
        if (err.has_value()) {
            return err;
        }
        if (this->last_instruction_is_pop()) {
            this->remove_last_pop();
        }
    }

    size_t after_alternative_position = this->ins.size();
    this->change_operand(jump_position, after_alternative_position);
    return std::nullopt;
}

std::optional<std::string>
compiler::compile_block(const block_statement& block) {
    return this->compile_statements(block.get_block());
}

} // namespace axe
