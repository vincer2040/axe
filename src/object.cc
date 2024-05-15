#include "object.h"
#include "base.h"

namespace axe {

object::object() : type(object_type::Null), data(std::monostate()) {}

object::object(object_type type, object_data data)
    : type(type), data(std::move(data)) {}

object_type object::get_type() const { return this->type; }

const char* const object_type_strings[] = {
    "Null", "Bool", "Integer", "Float", "String", "Error",
};

const char* object::type_to_strig() const {
    return object_type_strings[(int)this->type];
}

int64_t object::get_int() const {
    AXE_CHECK(this->type == object_type::Integer,
              "trying to get Integer from type %s",
              object_type_strings[(int)this->type]);
    return std::get<int64_t>(this->data);
}

double object::get_float() const {
    AXE_CHECK(this->type == object_type::Float,
              "trying to get Float from type %s",
              object_type_strings[(int)this->type]);
    return std::get<double>(this->data);
}

bool object::get_bool() const {
    AXE_CHECK(this->type == object_type::Bool,
              "trying to get Bool from type %s",
              object_type_strings[(int)this->type]);
    return std::get<bool>(this->data);
}

const std::string& object::get_string() const {
    AXE_CHECK(this->type == object_type::Float,
              "trying to get String from type %s",
              object_type_strings[(int)this->type]);
    return std::get<std::string>(this->data);
}

bool object::is_error() const { return this->type == object_type::Error; }

bool object::operator==(const object& other) const {
    if (this->type != other.type) {
        return false;
    }

    switch (this->type) {
    case object_type::Null:
        return true;
    case object_type::Bool:
        return this->get_bool() == other.get_bool();
    case object_type::Integer:
        return this->get_int() == other.get_int();
    case object_type::Float:
        return this->get_float() == other.get_float();
    case object_type::String:
        return this->get_string() == other.get_string();
    case object_type::Error:
        return false;
    }
    return false;
}

} // namespace axe