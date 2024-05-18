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

struct compilation_scope {
    instructions ins;
    emitted_instruction last_instruction;
    emitted_instruction previous_instruction;
};

using constants_owned = std::vector<object>;
using constants_ref = std::vector<object>&;

using symbol_table_owned = symbol_table;
using symbol_table_ref = symbol_table&;

template <typename ConstantsOwnership, typename SymbolTableOwnership>
class compiler {
  public:
    compiler();
    compiler(symbol_table_ref symb_table, constants_ref constants);
    std::optional<std::string> compile(const ast& ast);
    const byte_code get_byte_code() const;

  private:
    ConstantsOwnership constants;
    SymbolTableOwnership symb_table;
    std::vector<compilation_scope> scopes;
    size_t scope_index;

    const instructions& get_current_instructions() const;
    instructions& current_instructions();
    size_t emit(op_code op, const std::vector<int> operands);
    size_t add_instruction(const std::vector<uint8_t> ins);
    int add_constant(object obj);
    void set_last_instruction(op_code op, size_t position);
    bool last_instruction_is_pop();
    bool last_instruction_is(op_code op);
    void remove_last_pop();
    void replace_instruction(size_t position,
                             std::vector<uint8_t> new_instruction);
    void replace_last_pop_with_return();
    void change_operand(size_t op_position, int operand);

    void enter_scope();
    instructions leave_scope();

    std::optional<std::string>
    compile_statements(const std::vector<statement>& statements);
    std::optional<std::string> compile_statement(const statement& statement);
    std::optional<std::string> compile_let_statement(const let_statement& let);
    std::optional<std::string>
    compile_return_statement(const return_statement& ret);
    std::optional<std::string> compile_expression(const expression& expression);
    std::optional<std::string> compile_integer(int64_t value);
    std::optional<std::string> compile_string(const std::string& value);
    std::optional<std::string> compile_ident(const std::string& ident);
    std::optional<std::string> compile_prefix(const prefix& prefix);
    std::optional<std::string> compile_infix(const infix& infix);
    std::optional<std::string> compile_assignment(const assignment& assignment);
    std::optional<std::string> compile_if(const if_expression& if_exp);
    std::optional<std::string>
    compile_function(const function_expression& function);
    std::optional<std::string> compile_call(const call& call);

    std::optional<std::string> compile_block(const block_statement& block);
};

} // namespace axe

#endif // __AXE_COMPILER_H__
