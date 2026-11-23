#include "Environment.h"
#include "error.h"

void Environment::define(const std::string &name, const Value &val)
{
    values.insert_or_assign(name, val);
}

Value Environment::get(const Token &name) const
{
    auto it = values.find(name.lexeme);
    if (it != values.end()) return it->second;
    error(name, "Undefined variable.");
    throw RuntimeError{};
}

void Environment::assign(const Token &name, const Value &val)
{
    auto it = values.find(name.lexeme);
    if (it != values.end()) {
        it->second = val;
    } else {
        error(name, "Undefined variable.");
        throw RuntimeError{};
    }
}

