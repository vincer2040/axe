#ifndef __AXE_VM_H__

#define __AXE_VM_H__

#include "code.h"
#include "compiler.h"
#include "frame.h"
#include "object.h"
#include <vector>

#define STACK_SIZE 2048
#define GLOBALS_SIZE 65536
#define MAX_FRAMES 1024

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

    std::vector<frame> frames;
    size_t frames_index;

    object stack[STACK_SIZE];
    size_t stack_pointer;

    GlobalsLifeTime globals;

    frame& current_frame();
    void push_frame(frame frame);
    frame& pop_frame();

    std::optional<std::string> push(const object& obj);
    const object& pop();
};

} // namespace axe

#endif // __AXE_VM_H__
