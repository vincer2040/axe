#ifndef __AXE_VM_H__

#define __AXE_VM_H__

#include "code.h"
#include "compiler.h"
#include "object.h"
#include <vector>

#define STACK_SIZE 2048

namespace axe {

class vm {
    public:
        vm(byte_code byte_code);

        std::optional<std::string> run();
        std::optional<const object> stack_top();
        const object& last_popped_stack_element();
    private:
        std::vector<object> constants;
        instructions ins;
        object stack[STACK_SIZE];
        size_t stack_pointer;

        std::optional<std::string> push(object obj);
        const object& pop();
};

}

#endif // __AXE_VM_H__
