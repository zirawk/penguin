#ifndef VALUE_H
#define VALUE_H

#include <variant>
#include <string>

using Value = std::variant<std::monostate, bool, double, std::string>;

constexpr bool boolean(const Value &v)
{
    if (auto b = std::get_if<bool>(&v)) {
        return *b;
    } else if (std::holds_alternative<std::monostate>(v)) {
        return false;
    } else {
        return true;
    }
}


std::string stringify(const Value &v);

#endif // VALUE_H
