#ifndef __AXE_VM_H__

#define __AXE_VM_H__

#include "code.h"
#include "compiler.h"
#include "object.h"
#include <vector>

#define STACK_SIZE 2048
#define GLOBALS_SIZE 65536

namespace axe {

template <typename GlobalsLifeTime> class vm {
  public:
    vm(byte_code byte_code);
    vm(byte_code byte_code, GlobalsLifeTime globals);

    std::optional<std::string> run();
    std::optional<const object> stack_top();
    const object& last_popped_stack_element();

  private:
    std::vector<object> constants;
    instructions ins;
    object stack[STACK_SIZE];
    GlobalsLifeTime globals;
    size_t stack_pointer;

    std::optional<std::string> push(const object& obj);
    const object& pop();
};

} // namespace axe

#endif // __AXE_VM_H__
