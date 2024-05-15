#ifndef __AXE_CODE_H__

#define __AXE_CODE_H__

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace axe {

using instructions = std::vector<uint8_t>;

enum class op_code {
    OpConstant = 0,
};

class definition {
  public:
    definition(const char* name, const std::vector<int> operand_widths);
    const char* get_name() const;
    const std::vector<int>& get_operand_widths() const;

  private:
    const char* name;
    const std::vector<int> operand_widths;
};

std::optional<const definition> lookup(op_code op);

std::vector<uint8_t> make(op_code op, const std::vector<int> operands);

std::string instructions_string(const instructions& ins);

const std::pair<std::vector<int>, int> read_operands(const definition& def,
                                                     const instructions& ins);

} // namespace axe

#endif // __AXE_CODE_H__
