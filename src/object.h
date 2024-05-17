#ifndef __AXE_OBJECT_H__

#define __AXE_OBJECT_H__

#include "code.h"
#include <string>
#include <variant>

namespace axe {

enum class object_type {
    Null,
    Bool,
    Integer,
    Float,
    String,
    Error,
    Function,
};

class compiled_function {
  public:
    compiled_function();
    compiled_function(instructions ins, size_t num_locals);
    const instructions& get_instructions() const;
    size_t get_num_locals() const;

  private:
    instructions ins;
    size_t num_locals;
};

using object_data =
    std::variant<std::monostate, bool, int64_t, double, std::string, compiled_function>;

class object {
  public:
    object();
    object(object_type type, object_data data);

    object_type get_type() const;
    int64_t get_int() const;
    double get_float() const;
    bool get_bool() const;
    const std::string& get_string() const;
    const std::string& get_error() const;
    const compiled_function& get_function() const;

    const char* type_to_strig() const;
    std::string string() const;

    bool is_error() const;
    bool is_truthy() const;

    bool operator==(const object& other) const;
    bool operator>(const object& other) const;
    bool operator!=(const object& other) const;
    object operator+(const object& rhs) const;
    object operator-(const object& rhs) const;
    object operator*(const object& rhs) const;
    object operator/(const object& rhs) const;

  private:
    object_type type;
    object_data data;
};

} // namespace axe

#endif // __AXE_OBJECT_H__
