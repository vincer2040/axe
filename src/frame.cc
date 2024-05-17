#include "frame.h"
#include "object.h"

namespace axe {

frame::frame() : function(compiled_function()) {}

frame::frame(compiled_function function, size_t base_pointer)
    : function(function), instruction_pointer(-1), base_pointer(base_pointer) {}

const instructions& frame::get_instructions() const {
    return this->function.get_instructions();
}

} // namespace axe
