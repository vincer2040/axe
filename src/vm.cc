#include "vm.h"
#include "code.h"
#include <optional>

namespace axe {

template <>
vm<std::vector<object>>::vm(byte_code byte_code)
    : constants(std::move(byte_code.constants)),
      frames(std::vector<frame>(MAX_FRAMES, frame())),
      globals(std::vector<object>(GLOBALS_SIZE, object())), stack_pointer(0),
      frames_index(1) {
    auto main_fn = compiled_function(std::move(byte_code.ins), 0, 0);
    auto main_frame = frame(main_fn, 0);
    this->frames[0] = main_frame;
}

template <typename GlobalsLifeTime>
vm<GlobalsLifeTime>::vm(byte_code byte_code, GlobalsLifeTime globals)
    : constants(std::move(byte_code.constants)),
      frames(std::vector<frame>(MAX_FRAMES, frame())), globals(globals),
      stack_pointer(0), frames_index(1) {
    auto main_fn = compiled_function(std::move(byte_code.ins), 0, 0);
    auto main_frame = frame(main_fn, 0);
    this->frames[0] = main_frame;
}

template <typename GlobalsLifeTime>
frame& vm<GlobalsLifeTime>::current_frame() {
    return this->frames[this->frames_index - 1];
}

template <typename GlobalsLifeTime>
void vm<GlobalsLifeTime>::push_frame(frame frame) {
    this->frames[this->frames_index] = frame;
    this->frames_index++;
}

template <typename GlobalsLifeTime> frame& vm<GlobalsLifeTime>::pop_frame() {
    this->frames_index--;
    return this->frames[this->frames_index];
}

template <typename GlobalsLifeTime>
std::optional<std::string> vm<GlobalsLifeTime>::run() {
    std::optional<std::string> err = std::nullopt;
    while (this->current_frame().instruction_pointer <
           static_cast<ssize_t>(
               this->current_frame().get_instructions().size() - 1)) {
        this->current_frame().instruction_pointer++;
        size_t instruction_pointer = this->current_frame().instruction_pointer;
        auto& ins = this->current_frame().get_instructions();
        op_code op = static_cast<op_code>(ins[instruction_pointer]);
        switch (op) {
        case op_code::OpConstant: {
            uint16_t const_index = read_u16(ins, instruction_pointer + 1);
            err = this->push(this->constants[const_index]);
            if (err.has_value()) {
                return err;
            }
            this->current_frame().instruction_pointer += 2;
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
            size_t position =
                static_cast<size_t>(read_u16(ins, instruction_pointer + 1));
            this->current_frame().instruction_pointer = position - 1;
        } break;
        case op_code::OpJumpNotTruthy: {
            size_t position =
                static_cast<size_t>(read_u16(ins, instruction_pointer + 1));
            this->current_frame().instruction_pointer += 2;
            auto& condition = this->pop();
            if (!condition.is_truthy()) {
                this->current_frame().instruction_pointer = position - 1;
            }
        } break;
        case op_code::OpNull:
            err = this->push(object());
            break;
        case op_code::OpSetGlobal: {
            size_t global_index =
                static_cast<size_t>(read_u16(ins, instruction_pointer + 1));
            this->current_frame().instruction_pointer += 2;
            this->globals[global_index] = this->pop();
        } break;
        case op_code::OpGetGlobal: {
            size_t global_index =
                static_cast<size_t>(read_u16(ins, instruction_pointer + 1));
            this->current_frame().instruction_pointer += 2;
            err = this->push(this->globals[global_index]);
        } break;
        case op_code::OpCall: {
            size_t num_args = ins[instruction_pointer + 1];
            this->current_frame().instruction_pointer++;

            err = this->call_function(num_args);
        } break;
        case op_code::OpReturnValue: {
            auto return_value = this->pop();
            auto& frame = this->pop_frame();
            this->stack_pointer = frame.base_pointer - 1;
            err = this->push(return_value);
        } break;
        case op_code::OpReturn: {
            auto& frame = this->pop_frame();
            this->stack_pointer = frame.base_pointer - 1;
            err = this->push(object());
        } break;
        case op_code::OpSetLocal: {
            size_t local_index = ins[instruction_pointer + 1];
            this->current_frame().instruction_pointer += 1;
            auto& frame = this->current_frame();
            this->stack[frame.base_pointer + local_index] = this->pop();
        } break;
        case op_code::OpGetLocal: {
            size_t local_index = ins[instruction_pointer + 1];
            this->current_frame().instruction_pointer += 1;
            auto& frame = this->current_frame();
            err = this->push(this->stack[frame.base_pointer + local_index]);
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

template <typename GlobalsLifeTime>
std::optional<std::string> vm<GlobalsLifeTime>::call_function(size_t num_args) {
    auto& fn_obj = this->stack[this->stack_pointer - 1 - num_args];
    if (fn_obj.get_type() != object_type::Function) {
        return "calling non-function";
    }
    auto& fn = fn_obj.get_function();
    if (fn.get_num_params() != num_args) {
        return "wrong number of arguments: want " +
               std::to_string(fn.get_num_params()) + ", got " +
               std::to_string(num_args);
    }
    frame frame(fn, this->stack_pointer - num_args);
    this->push_frame(frame);
    this->stack_pointer = frame.base_pointer + fn.get_num_locals();
    return std::nullopt;
}

template class vm<std::vector<object>>;
template class vm<std::vector<object>&>;

} // namespace axe
