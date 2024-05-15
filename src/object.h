#ifndef __AXE_OBJECT_H__

#define __AXE_OBJECT_H__

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
};

using object_data =
    std::variant<std::monostate, bool, int64_t, double, std::string>;

class object {
  public:
    object();
    object(object_type type, object_data data);

    object_type get_type() const;
    int64_t get_int() const;
    double get_float() const;
    bool get_bool() const;
    const std::string& get_string() const;

    const char* type_to_strig() const;
    std::string string();

    bool is_error() const;

    bool operator==(const object& other) const;

  private:
    object_type type;
    object_data data;
};

} // namespace axe

#endif // __AXE_OBJECT_H__
