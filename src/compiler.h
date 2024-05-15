#ifndef __AXE_COMPILER_H__

#define __AXE_COMPILER_H__

#include "ast.h"
#include "code.h"
#include "object.h"

namespace axe {

struct byte_code {
    const instructions& ins;
    const std::vector<object>& constants;
};

class compiler {
  public:
    std::optional<std::string> compile(const ast& ast);
    const byte_code get_byte_code() const;

  private:
    instructions ins;
    std::vector<object> constants;

    int emit(op_code op, const std::vector<int> operands);
    int add_instruction(const std::vector<uint8_t> ins);
    int add_constant(object obj);

    std::optional<std::string>
    compile_statements(const std::vector<statement>& statements);
    std::optional<std::string> compile_statement(const statement& statement);
    std::optional<std::string> compile_expression(const expression& expression);
    std::optional<std::string> compile_infix(const infix& infix);
    std::optional<std::string> compile_integer(int64_t value);
};

} // namespace axe

#endif // __AXE_COMPILER_H__
