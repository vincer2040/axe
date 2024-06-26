#include "object.h"
#include "base.h"

namespace axe {

compiled_function::compiled_function()
    : ins(std::vector<uint8_t>()), num_locals(0), num_params(0) {}

compiled_function::compiled_function(instructions ins, size_t num_locals,
                                     size_t num_params)
    : ins(ins), num_locals(num_locals), num_params(num_params) {}

const instructions& compiled_function::get_instructions() const {
    return this->ins;
}

size_t compiled_function::get_num_locals() const { return this->num_locals; }

size_t compiled_function::get_num_params() const { return this->num_params; }

object::object() : type(object_type::Null), data(std::monostate()) {}

object::object(object_type type, object_data data)
    : type(type), data(std::move(data)) {}

object_type object::get_type() const { return this->type; }

const char* const object_type_strings[] = {
    "Null", "Bool", "Integer", "Float", "String", "Error", "Function",
};

const char* object::type_to_string() const {
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
    AXE_CHECK(this->type == object_type::String,
              "trying to get String from type %s",
              object_type_strings[(int)this->type]);
    return std::get<std::string>(this->data);
}

const std::string& object::get_error() const {
    AXE_CHECK(this->type == object_type::Error,
              "trying to get Error from type %s",
              object_type_strings[(int)this->type]);
    return std::get<std::string>(this->data);
}

const compiled_function& object::get_function() const {
    AXE_CHECK(this->type == object_type::Function,
              "trying to get Function from type %s",
              object_type_strings[(int)this->type]);
    return std::get<compiled_function>(this->data);
}

std::string object::string() const {
    std::string res;
    switch (this->type) {
    case object_type::Null:
        res += "Null";
        break;
    case object_type::Bool:
        res += this->get_bool() ? "true" : "false";
        break;
    case object_type::Integer:
        res += std::to_string(this->get_int());
        break;
    case object_type::Float:
        res += std::to_string(this->get_float());
        break;
    case object_type::String:
        res += "\"" + this->get_string() + "\"";
        break;
    case object_type::Error:
        res += "ERROR: " + this->get_error();
        break;
    case object_type::Function:
        res += "function";
        break;
    }
    return res;
}

bool object::is_error() const { return this->type == object_type::Error; }

bool object::is_truthy() const {
    switch (this->type) {
    case object_type::Bool:
        return this->get_bool();
    case object_type::Integer:
        return this->get_int() ? true : false;
    case object_type::Float:
        return this->get_float() ? true : false;
    case object_type::String:
        return true;
    case object_type::Error:
        return false;
    default:
        break;
    }
    return false;
}

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
    case object_type::Function: {
        // TODO: probably not this
        auto& this_func = this->get_function();
        auto& other_func = other.get_function();
        auto& this_ins = this_func.get_instructions();
        auto& other_ins = other_func.get_instructions();
        if (this_func.get_num_params() != other_func.get_num_params()) {
            return false;
        }
        if (this_func.get_num_locals() != other_func.get_num_locals()) {
            return false;
        }
        if (this_ins.size() != other_ins.size()) {
            return false;
        }
        for (size_t i = 0; i < this_ins.size(); ++i) {
            if (this_ins[i] != other_ins[i]) {
                return false;
            }
        }
        return true;
    };
    default:
        break;
    }
    return false;
}

bool object::operator!=(const object& other) const {
    if (this->type != other.type) {
        return true;
    }

    switch (this->type) {
    case object_type::Null:
        return false;
    case object_type::Bool:
        return this->get_bool() != other.get_bool();
    case object_type::Integer:
        return this->get_int() != other.get_int();
    case object_type::Float:
        return this->get_float() != other.get_float();
    case object_type::String:
        return this->get_string() != other.get_string();
    case object_type::Error:
        return true;
    default:
        break;
    }
    return true;
}

bool object::operator>(const object& rhs) const {
    if (this->type == object_type::Integer &&
        rhs.type == object_type::Integer) {
        return this->get_int() > rhs.get_int();
    }
    if (this->type == object_type::Float && rhs.type == object_type::Float) {
        return this->get_float() > rhs.get_float();
    }
    return false;
}

object object::operator+(const object& rhs) const {
    if (this->type != rhs.type) {
        return object();
    }
    switch (this->type) {
    case object_type::Integer:
        return object(object_type::Integer, this->get_int() + rhs.get_int());
    case object_type::Float:
        return object(object_type::Float, this->get_float() + rhs.get_float());
    case object_type::String:
        return object(object_type::String,
                      this->get_string() + rhs.get_string());
    default:
        break;
    }
    return object();
}

object object::operator-(const object& rhs) const {
    if (this->type == object_type::Integer &&
        rhs.type == object_type::Integer) {
        return object(object_type::Integer, this->get_int() - rhs.get_int());
    }
    if (this->type == object_type::Float &&
        rhs.get_type() == object_type::Float) {
        return object(object_type::Float, this->get_float() - rhs.get_float());
    }
    return object();
}

object object::operator*(const object& rhs) const {
    if (this->type == object_type::Integer &&
        rhs.type == object_type::Integer) {
        return object(object_type::Integer, this->get_int() * rhs.get_int());
    }
    if (this->type == object_type::Float &&
        rhs.get_type() == object_type::Float) {
        return object(object_type::Float, this->get_float() * rhs.get_float());
    }
    return object();
}

object object::operator/(const object& rhs) const {
    if (this->type == object_type::Integer &&
        rhs.type == object_type::Integer) {
        return object(object_type::Integer, this->get_int() / rhs.get_int());
    }
    if (this->type == object_type::Float &&
        rhs.get_type() == object_type::Float) {
        return object(object_type::Float, this->get_float() / rhs.get_float());
    }
    return object();
}

} // namespace axe
