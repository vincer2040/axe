#include "vm.h"
#include "code.h"
#include <optional>

namespace axe {

template <>
vm<std::vector<object>>::vm(byte_code byte_code)
    : constants(std::move(byte_code.constants)), ins(std::move(byte_code.ins)),
      globals(std::vector<object>(GLOBALS_SIZE, object())), stack_pointer(0) {}

template <typename GlobalsLifeTime>
vm<GlobalsLifeTime>::vm(byte_code byte_code, GlobalsLifeTime globals)
    : constants(std::move(byte_code.constants)), ins(std::move(byte_code.ins)),
      globals(globals), stack_pointer(0) {}

template <typename GlobalsLifeTime>
std::optional<std::string> vm<GlobalsLifeTime>::run() {
    std::optional<std::string> err = std::nullopt;
    for (size_t instruction_pointer = 0; instruction_pointer < this->ins.size();
         ++instruction_pointer) {
        op_code op = static_cast<op_code>(this->ins[instruction_pointer]);
        switch (op) {
        case op_code::OpConstant: {
            uint16_t const_index = read_u16(this->ins, instruction_pointer + 1);
            err = this->push(this->constants[const_index]);
            if (err.has_value()) {
                return err;
            }
            instruction_pointer += 2;
        } break;
        case op_code::OpAdd: {
            auto& rhs = this->pop();
            auto& lhs = this->pop();
            err = this->push(lhs + rhs);
        } break;
        case op_code::OpPop:
            this->pop();
            break;
        case op_code::OpSub: {
            auto& rhs = this->pop();
            auto& lhs = this->pop();
            err = this->push(lhs - rhs);
        } break;
        case op_code::OpMul: {
            auto& rhs = this->pop();
            auto& lhs = this->pop();
            err = this->push(lhs * rhs);
        } break;
        case op_code::OpDiv: {
            auto& rhs = this->pop();
            auto& lhs = this->pop();
            err = this->push(lhs / rhs);
        } break;
        case op_code::OpTrue:
            err = this->push(object(object_type::Bool, true));
            break;
        case op_code::OpFalse:
            err = this->push(object(object_type::Bool, false));
            break;
        case op_code::OpEq: {
            auto& rhs = this->pop();
            auto& lhs = this->pop();
            err = this->push(object(object_type::Bool, lhs == rhs));
        } break;
        case op_code::OpNotEq: {
            auto& rhs = this->pop();
            auto& lhs = this->pop();
            err = this->push(object(object_type::Bool, lhs != rhs));
        } break;
        case op_code::OpGreaterThan: {
            auto& rhs = this->pop();
            auto& lhs = this->pop();
            err = this->push(object(object_type::Bool, lhs > rhs));
        } break;
        case op_code::OpBang: {
            auto& rhs = this->pop();
            err = this->push(object(object_type::Bool, !rhs.is_truthy()));
        } break;
        case op_code::OpMinus: {
            auto& rhs = this->pop();
            if (rhs.get_type() != object_type::Integer) {
                err = "unsupported type for negation " +
                      std::string(rhs.type_to_strig());
                return err;
            }
            err = this->push(object(object_type::Integer, -rhs.get_int()));
        } break;
        case op_code::OpJump: {
            size_t position = static_cast<size_t>(
                read_u16(this->ins, instruction_pointer + 1));
            instruction_pointer = position - 1;
        } break;
        case op_code::OpJumpNotTruthy: {
            size_t position = static_cast<size_t>(
                read_u16(this->ins, instruction_pointer + 1));
            instruction_pointer += 2;
            auto& condition = this->pop();
            if (!condition.is_truthy()) {
                instruction_pointer = position - 1;
            }
        } break;
        case op_code::OpNull:
            err = this->push(object());
            break;
        case op_code::OpSetGlobal: {
            size_t global_index = static_cast<size_t>(
                read_u16(this->ins, instruction_pointer + 1));
            instruction_pointer += 2;
            this->globals[global_index] = this->pop();
        } break;
        case op_code::OpGetGlobal: {
            size_t global_index = static_cast<size_t>(
                read_u16(this->ins, instruction_pointer + 1));
            instruction_pointer += 2;
            err = this->push(this->globals[global_index]);
        } break;
        }
    }
    return err;
}

template <typename GlobalsLifeTime>
std::optional<const object> vm<GlobalsLifeTime>::stack_top() {
    if (this->stack_pointer == 0) {
        return std::nullopt;
    }
    return this->stack[this->stack_pointer - 1];
}

template <typename GlobalsLifeTime>
const object& vm<GlobalsLifeTime>::last_popped_stack_element() {
    return this->stack[this->stack_pointer];
}

template <typename GlobalsLifeTime>
std::optional<std::string> vm<GlobalsLifeTime>::push(const object& obj) {
    if (this->stack_pointer >= STACK_SIZE) {
        return "stack overflow";
    }
    this->stack[this->stack_pointer] = obj;
    this->stack_pointer++;
    return std::nullopt;
}

template <typename GlobalsLifeTime> const object& vm<GlobalsLifeTime>::pop() {
    auto& res = this->stack[this->stack_pointer - 1];
    this->stack_pointer--;
    return res;
}

template class vm<std::vector<object>>;
template class vm<std::vector<object>&>;

} // namespace axe
