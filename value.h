#ifndef VALUE_H
#define VALUE_H

#include <variant>
#include <string>

using Value = std::variant<std::monostate, bool, double, std::string>;

std::string stringify(const Value &v);

#endif
