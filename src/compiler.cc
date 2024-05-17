#include "compiler.h"
#include "ast.h"
#include "base.h"
#include "code.h"
#include <optional>

namespace axe {

template <>
compiler<constants_owned, symbol_table_owned>::compiler()
    : symb_table(symbol_table()), scope_index(0) {
    compilation_scope main_scope = {std::vector<uint8_t>(), {}, {}};
    this->scopes.push_back(main_scope);
}

template <>
compiler<constants_ref, symbol_table_ref>::compiler(
    symbol_table& symb_table, std::vector<object>& constants)
    : symb_table(symb_table), constants(constants), scope_index(0) {
    compilation_scope main_scope = {std::vector<uint8_t>(), {}, {}};
    this->scopes.push_back(main_scope);
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile(const ast& ast) {
    return this->compile_statements(ast.get_statements());
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
const byte_code
compiler<ConstantsLifeTime, SymbolTableLifeTime>::get_byte_code() const {
    return {this->get_current_instructions(), this->constants};
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
instructions&
compiler<ConstantsLifeTime, SymbolTableLifeTime>::current_instructions() {
    return this->scopes[this->scope_index].ins;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
const instructions&
compiler<ConstantsLifeTime, SymbolTableLifeTime>::get_current_instructions()
    const {
    return this->scopes[this->scope_index].ins;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
size_t compiler<ConstantsLifeTime, SymbolTableLifeTime>::emit(
    op_code op, const std::vector<int> operands) {
    auto instructions = make(op, operands);
    int pos = this->add_instruction(instructions);
    this->set_last_instruction(op, pos);
    return pos;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
size_t compiler<ConstantsLifeTime, SymbolTableLifeTime>::add_instruction(
    const std::vector<uint8_t> ins) {
    size_t pos_new_instruction = this->current_instructions().size();
    this->current_instructions().insert(this->current_instructions().end(),
                                        ins.begin(), ins.end());
    return pos_new_instruction;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
int compiler<ConstantsLifeTime, SymbolTableLifeTime>::add_constant(object obj) {
    this->constants.push_back(std::move(obj));
    return this->constants.size() - 1;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
void compiler<ConstantsLifeTime, SymbolTableLifeTime>::set_last_instruction(
    op_code op, size_t position) {
    emitted_instruction last = {op, position};
    emitted_instruction previous =
        this->scopes[this->scope_index].last_instruction;
    this->scopes[this->scope_index].previous_instruction = previous;
    this->scopes[this->scope_index].last_instruction = last;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
bool compiler<ConstantsLifeTime,
              SymbolTableLifeTime>::last_instruction_is_pop() {
    return this->scopes[this->scope_index].last_instruction.op ==
           op_code::OpPop;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
bool compiler<ConstantsLifeTime, SymbolTableLifeTime>::last_instruction_is(
    op_code op) {
    if (this->current_instructions().size() == 0) {
        return false;
    }
    return this->scopes[this->scope_index].last_instruction.op == op;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
void compiler<ConstantsLifeTime, SymbolTableLifeTime>::remove_last_pop() {
    auto previous = this->scopes[this->scope_index].previous_instruction;
    this->current_instructions().pop_back();
    this->scopes[this->scope_index].last_instruction = previous;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
void compiler<ConstantsLifeTime, SymbolTableLifeTime>::replace_instruction(
    size_t position, std::vector<uint8_t> new_instruction) {
    auto& ins = this->current_instructions();
    for (size_t i = 0; i < new_instruction.size(); ++i) {
        ins[position + i] = new_instruction[i];
    }
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
void compiler<ConstantsLifeTime,
              SymbolTableLifeTime>::replace_last_pop_with_return() {
    size_t last_position =
        this->scopes[this->scope_index].last_instruction.position;
    this->replace_instruction(last_position,
                              axe::make(op_code::OpReturnValue, {}));
    this->scopes[this->scope_index].last_instruction.op =
        op_code::OpReturnValue;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
void compiler<ConstantsLifeTime, SymbolTableLifeTime>::change_operand(
    size_t op_position, int operand) {
    op_code op =
        static_cast<op_code>(this->current_instructions()[op_position]);
    auto new_instruction = make(op, {operand});
    this->replace_instruction(op_position, new_instruction);
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
void compiler<ConstantsLifeTime, SymbolTableLifeTime>::enter_scope() {
    compilation_scope scope;
    this->scopes.push_back(scope);
    this->scope_index++;
    this->symb_table = symbol_table::with_outer(this->symb_table);
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
instructions compiler<ConstantsLifeTime, SymbolTableLifeTime>::leave_scope() {
    instructions ins = this->current_instructions();
    this->scopes.pop_back();
    this->scope_index--;
    this->symb_table = this->symb_table.get_outer();
    return ins;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_statements(
    const std::vector<statement>& statements) {
    for (auto& statement : statements) {
        auto err = this->compile_statement(statement);
        if (err.has_value()) {
            return err;
        }
    }
    return std::nullopt;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_statement(
    const statement& statement) {
    std::optional<std::string> err = std::nullopt;
    switch (statement.get_type()) {
    case statement_type::LetStatement:
        err = this->compile_let_statement(statement.get_let());
        break;
    case statement_type::ReturnStatement:
        err = this->compile_return_statement(statement.get_return());
        break;
    case statement_type::ExpressionStatement:
        err = this->compile_expression(statement.get_expression());
        // current work around for setting functions
        if (!this->last_instruction_is(op_code::OpSetGlobal)) {
            this->emit(op_code::OpPop, {});
        }
        break;
    default:
        AXE_UNREACHABLE;
    }
    return err;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_let_statement(
    const let_statement& let) {
    auto err = this->compile_expression(let.get_value());
    if (err.has_value()) {
        return err;
    }
    auto symbol = this->symb_table.define(let.get_name());
    if (symbol.scope == axe::symbol_scope::GlobalScope) {
        this->emit(op_code::OpSetGlobal, {(int)symbol.index});
    } else {
        this->emit(op_code::OpSetLocal, {(int)symbol.index});
    }
    return std::nullopt;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_return_statement(
    const return_statement& ret) {
    auto err = this->compile_expression(ret);
    if (err.has_value()) {
        return err;
    }
    this->emit(op_code::OpReturnValue, {});
    return std::nullopt;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_expression(
    const expression& expression) {
    std::optional<std::string> err = std::nullopt;
    switch (expression.get_type()) {
    case expression_type::Integer:
        err = this->compile_integer(expression.get_int());
        break;
    case expression_type::Bool:
        this->emit(expression.get_bool() ? op_code::OpTrue : op_code::OpFalse,
                   {});
        break;
    case expression_type::String:
        err = this->compile_string(expression.get_string());
        break;
    case expression_type::Ident:
        err = this->compile_ident(expression.get_ident());
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
    case expression_type::Function:
        err = this->compile_function(expression.get_function());
        break;
    case expression_type::Call:
        err = this->compile_call(expression.get_call());
        break;
    default:
        err = "cannot compile " + std::string(expression.get_type_string());
        break;
    }
    return err;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_integer(
    int64_t value) {
    object obj(object_type::Integer, value);
    this->emit(op_code::OpConstant, {this->add_constant(std::move(obj))});
    return std::nullopt;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_string(
    const std::string& value) {
    object obj(object_type::String, value);
    this->emit(op_code::OpConstant, {this->add_constant(std::move(obj))});
    return std::nullopt;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_ident(
    const std::string& ident) {
    auto symbol = this->symb_table.resolve(ident);
    if (!symbol.has_value()) {
        return "undefined variable " + ident;
    }
    if (symbol->scope == axe::symbol_scope::GlobalScope) {
        this->emit(op_code::OpGetGlobal, {(int)symbol->index});
    } else {
        this->emit(op_code::OpGetLocal, {(int)symbol->index});
    }
    return std::nullopt;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_prefix(
    const prefix& prefix) {
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

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_infix(
    const infix& infix) {

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

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_if(
    const if_expression& if_exp) {
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
    if (this->last_instruction_is(op_code::OpPop)) {
        this->remove_last_pop();
    }

    size_t jump_position = this->emit(op_code::OpJump, {9999});
    size_t after_consequence_position = this->current_instructions().size();
    this->change_operand(jump_not_truthy_position, after_consequence_position);

    auto& alternative = if_exp.get_alternative();
    if (!alternative.has_value()) {
        this->emit(op_code::OpNull, {});
    } else {
        err = this->compile_block(*alternative);
        if (err.has_value()) {
            return err;
        }
        if (this->last_instruction_is(op_code::OpPop)) {
            this->remove_last_pop();
        }
    }

    size_t after_alternative_position = this->current_instructions().size();
    this->change_operand(jump_position, after_alternative_position);
    return std::nullopt;
}

template <typename ConstantsLifeTime, typename SymbolTableLIfeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLIfeTime>::compile_function(
    const function_expression& function) {
    this->enter_scope();
    auto err = this->compile_block(function.get_body());
    if (err.has_value()) {
        return err;
    }
    if (this->last_instruction_is(op_code::OpPop)) {
        this->replace_last_pop_with_return();
    }
    if (!this->last_instruction_is(op_code::OpReturnValue)) {
        this->emit(op_code::OpReturn, {});
    }
    size_t num_locals = this->symb_table.get_num_definitions();
    instructions ins = this->leave_scope();
    object obj(object_type::Function,
               compiled_function(std::move(ins), num_locals));
    this->emit(op_code::OpConstant, {this->add_constant(std::move(obj))});
    auto& name = function.get_name();
    if (name.has_value()) {
        auto symbol = this->symb_table.define(*name);
        this->emit(op_code::OpSetGlobal, {(int)symbol.index});
    }
    return std::nullopt;
}

template <typename ConstantsLifeTime, typename SymbolTableLIfeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLIfeTime>::compile_call(
    const call& call) {
    auto err = this->compile_expression(*call.get_function());
    if (err.has_value()) {
        return err;
    }
    this->emit(op_code::OpCall, {});
    return std::nullopt;
}

template <typename ConstantsLifeTime, typename SymbolTableLifeTime>
std::optional<std::string>
compiler<ConstantsLifeTime, SymbolTableLifeTime>::compile_block(
    const block_statement& block) {
    return this->compile_statements(block.get_block());
}

template class compiler<constants_owned, symbol_table_owned>;
template class compiler<constants_ref, symbol_table_ref>;

} // namespace axe
