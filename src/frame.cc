#include "frame.h"
#include "object.h"

namespace axe {

frame::frame() : function(compiled_function()) {}

frame::frame(compiled_function function)
    : function(function), instruction_pointer(-1) {}

const instructions& frame::get_instructions() const {
    return this->function.get_instructions();
}

} // namespace axe
