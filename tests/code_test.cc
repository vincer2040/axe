#include "../src/code.h"
#include <gtest/gtest.h>

struct make_test {
    axe::op_code op;
    std::vector<int> operands;
    std::vector<uint8_t> expected;
};

TEST(Code, Make) {
    make_test tests[] = {
        {axe::op_code::OpConstant,
         {65534},
         {(uint8_t)axe::op_code::OpConstant, 255, 254}},
        {axe::op_code::OpAdd, {}, {(uint8_t)axe::op_code::OpAdd}},
        {axe::op_code::OpGetLocal,
         {255},
         {(uint8_t)axe::op_code::OpGetLocal, 255}},
    };

    for (auto& test : tests) {
        auto instruction = axe::make(test.op, test.operands);
        EXPECT_EQ(instruction.size(), test.expected.size());
        for (size_t i = 0; i < test.expected.size(); ++i) {
            EXPECT_EQ(instruction[i], test.expected[i]);
        }
    }
}

TEST(Code, InstructionString) {
    axe::instructions instructions[] = {
        axe::make(axe::op_code::OpAdd, {}),
        axe::make(axe::op_code::OpGetLocal, {1}),
        axe::make(axe::op_code::OpConstant, {2}),
        axe::make(axe::op_code::OpConstant, {65535}),
    };

    std::string expected = "\
0000 OpAdd\n\
0001 OpGetLocal 1\n\
0003 OpConstant 2\n\
0006 OpConstant 65535\n\
";
    axe::instructions concatted;
    for (auto& ins : instructions) {
        concatted.insert(concatted.end(), ins.begin(), ins.end());
    }
    auto str = axe::instructions_string(concatted);
    EXPECT_STREQ(expected.c_str(), str.c_str());
}

struct read_operands_test {
    axe::op_code op;
    std::vector<int> operands;
    int bytes_read;
};

TEST(Code, ReadOperands) {
    read_operands_test tests[] = {
        {axe::op_code::OpConstant, {65535}, 2},
        {axe::op_code::OpGetLocal, {255}, 1},
    };
    for (auto& test : tests) {
        auto instruction = axe::make(test.op, test.operands);
        auto def = axe::lookup(test.op);
        EXPECT_TRUE(def.has_value());
        instruction.erase(instruction.begin());
        auto& read = axe::read_operands(*def, instruction);
        auto& operands_read = read.first;
        int n = read.second;
        EXPECT_EQ(n, test.bytes_read);
        for (size_t i = 0; i < test.operands.size(); ++i) {
            EXPECT_EQ(test.operands[i], operands_read[i]);
        }
    }
}
