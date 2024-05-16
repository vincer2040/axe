#include "code.h"
#include <optional>
#include <stdint.h>
#include <unordered_map>

namespace axe {

definition::definition(const char* name, const std::vector<int> operand_widths)
    : name(name), operand_widths(std::move(operand_widths)) {}

const char* definition::get_name() const { return this->name; }

const std::vector<int>& definition::get_operand_widths() const {
    return this->operand_widths;
}

static const std::unordered_map<op_code, definition> definitions = {
    {op_code::OpConstant, definition("OpConstant", {2})},
    {op_code::OpAdd, definition("OpAdd", {})},
    {op_code::OpPop, definition("OpPop", {})},
    {op_code::OpSub, definition("OpSub", {})},
    {op_code::OpMul, definition("OpMul", {})},
    {op_code::OpDiv, definition("OpDiv", {})},
    {op_code::OpTrue, definition("OpTrue", {})},
    {op_code::OpFalse, definition("OpFalse", {})},
};

std::optional<const definition> lookup(op_code op) {
    const auto& it = definitions.find(op);
    if (it == definitions.end()) {
        return std::nullopt;
    }
    return it->second;
}

static void put_big_endian_u16(std::vector<uint8_t>& instruction,
                               uint16_t operand) {
    instruction.push_back(static_cast<uint8_t>(operand >> 8));
    instruction.push_back(static_cast<uint8_t>(operand));
}

std::vector<uint8_t> make(op_code op, const std::vector<int> operands) {
    auto def = lookup(op);
    if (!def.has_value()) {
        return {};
    }
    int instruction_length = 1;
    auto& operand_widths = def->get_operand_widths();
    for (auto& width : operand_widths) {
        instruction_length += width;
    }
    std::vector<uint8_t> instruction;
    instruction.reserve(instruction_length);
    instruction.push_back((uint8_t)op);
    for (size_t i = 0; i < operands.size(); ++i) {
        int operand = operands[i];
        int width = operand_widths[i];
        switch (width) {
        case 2:
            put_big_endian_u16(instruction, operand);
            break;
        }
    }
    return instruction;
}

std::string format_instructions(const definition& def,
                                const std::vector<int> operands) {
    std::string res;
    auto& operand_widths = def.get_operand_widths();
    size_t operand_count = operand_widths.size();
    if (operands.size() != operand_count) {
        res += "ERROR: operand len " + std::to_string(operands.size()) +
               " does not match defined " + std::to_string(operand_count);
        return res;
    }
    switch (operand_count) {
    case 0:
        return std::string(def.get_name());
    case 1:
        res += std::string(def.get_name()) + " " + std::to_string(operands[0]);
        return res;
    }
    res += "ERROR: unhandled operand_count for " + std::string(def.get_name()) +
           "\n";
    return res;
}

std::string instructions_string(const instructions& ins) {
    std::string res;
    size_t i = 0;
    while (i < ins.size()) {
        auto def = lookup(static_cast<op_code>(ins[i]));
        if (!def.has_value()) {
            std::string err =
                "ERROR: opcode " + std::to_string(ins[i]) + " undefined";
            continue;
        }

        auto ins_slice = std::vector<uint8_t>(ins.begin() + i + 1, ins.end());
        auto& [operands, read] = read_operands(*def, ins_slice);
        char num_buf[5] = {0};
        snprintf(num_buf, 5, "%04d", (int)i);
        res += num_buf;
        res += ' ';
        res += format_instructions(*def, operands);
        res += '\n';

        i += 1 + read;
    }
    return res;
}

uint16_t read_u16(const instructions& ins, int offset) {
    uint16_t res = static_cast<uint16_t>(ins[offset] << 8);
    offset++;
    res |= static_cast<uint16_t>(ins[offset]);
    return res;
}

const std::pair<std::vector<int>, int> read_operands(const definition& def,
                                                     const instructions& ins) {
    std::vector<int> operands;
    auto& operand_widths = def.get_operand_widths();
    operands.reserve(operand_widths.size());
    int offset = 0;
    for (auto& width : operand_widths) {
        switch (width) {
        case 2:
            operands.push_back(read_u16(ins, offset));
            break;
        }
        offset += width;
    }
    return std::pair<std::vector<int>, int>(operands, offset);
}

} // namespace axe
