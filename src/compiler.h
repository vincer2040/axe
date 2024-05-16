#ifndef __AXE_COMPILER_H__

#define __AXE_COMPILER_H__

#include "ast.h"
#include "code.h"
#include "object.h"
#include "symbol_table.h"

namespace axe {

struct byte_code {
    const instructions& ins;
    const std::vector<object>& constants;
};

struct emitted_instruction {
    op_code op;
    size_t position;
};

class compiler {
  public:
    compiler() = delete;
    compiler(symbol_table& symb_table, std::vector<object>& constants);
    std::optional<std::string> compile(const ast& ast);
    const byte_code get_byte_code() const;

  private:
    instructions ins;
    std::vector<object>& constants;
    emitted_instruction last_instruction;
    emitted_instruction previous_instruction;
    symbol_table& symb_table;

    size_t emit(op_code op, const std::vector<int> operands);
    size_t add_instruction(const std::vector<uint8_t> ins);
    int add_constant(object obj);
    void set_last_instruction(op_code op, size_t position);
    bool last_instruction_is_pop();
    void remove_last_pop();
    void replace_instruction(size_t position,
                             std::vector<uint8_t> new_instruction);
    void change_operand(size_t op_position, int operand);

    std::optional<std::string>
    compile_statements(const std::vector<statement>& statements);
    std::optional<std::string> compile_statement(const statement& statement);
    std::optional<std::string> compile_let_statement(const let_statement& let);
    std::optional<std::string> compile_expression(const expression& expression);
    std::optional<std::string> compile_integer(int64_t value);
    std::optional<std::string> compile_ident(const std::string& ident);
    std::optional<std::string> compile_prefix(const prefix& prefix);
    std::optional<std::string> compile_infix(const infix& infix);
    std::optional<std::string> compile_if(const if_expression& if_exp);

    std::optional<std::string> compile_block(const block_statement& block);
};

} // namespace axe

#endif // __AXE_COMPILER_H__
