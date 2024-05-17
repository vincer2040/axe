#ifndef __AXE_FRAME_H__

#define __AXE_FRAME_H__

#include "object.h"

namespace axe {

class frame {
  public:
    frame();
    frame(compiled_function function);

    ssize_t instruction_pointer;

    const instructions& get_instructions() const;

  private:
    compiled_function function;
};

} // namespace axe

#endif // __AXE_FRAME_H__
