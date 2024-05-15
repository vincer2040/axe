#include "vm.h"
#include "code.h"

namespace axe {

vm::vm(byte_code byte_code)
    : constants(std::move(byte_code.constants)), ins(std::move(byte_code.ins)),
      stack_pointer(0) {}

std::optional<std::string> vm::run() {
    for (size_t instruction_pointer = 0; instruction_pointer < this->ins.size();
         ++instruction_pointer) {
        op_code op = static_cast<op_code>(this->ins[instruction_pointer]);
        switch (op) {
        case op_code::OpConstant: {
            uint16_t const_index = read_u16(this->ins, instruction_pointer + 1);
            auto err = this->push(this->constants[const_index]);
            if (err.has_value()) {
                return err;
            }
            instruction_pointer += 2;
        } break;
        case op_code::OpAdd: {
            auto& rhs = this->pop();
            auto& lhs = this->pop();
            this->push(lhs + rhs);
        } break;
        }
    }
    return std::nullopt;
}

std::optional<const object> vm::stack_top() {
    if (this->stack_pointer == 0) {
        return std::nullopt;
    }
    return this->stack[this->stack_pointer - 1];
}

std::optional<std::string> vm::push(object obj) {
    if (this->stack_pointer >= STACK_SIZE) {
        return "stack overflow";
    }
    this->stack[this->stack_pointer] = obj;
    this->stack_pointer++;
    return std::nullopt;
}

const object& vm::pop() {
    auto& res = this->stack[this->stack_pointer - 1];
    this->stack_pointer--;
    return res;
}

} // namespace axe
