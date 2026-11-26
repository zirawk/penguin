#include "Environment.h"
#include "error.h"
#include <memory>

Environment::Environment() : outer(nullptr) {}

Environment::Environment(std::shared_ptr<Environment> out) : outer(out) {}

Environment *Environment::ancestor(int distance)
{
    auto env = this;
    for (int i = 0; i < distance; ++i) {
        env = env->outer.get();
    }
    return env;
}

void Environment::define(const std::string &name, const Value &val)
{
    values.insert_or_assign(name, val);
}

Value Environment::get(const Token &name)
{
    auto it = values.find(name.lexeme);
    if (it != values.end()) return it->second;

    if (outer != nullptr) return outer->get(name);

    error(name, "Undefined variable.");
    throw RuntimeError{};
}

// 调用此函数应保证对应变量已定义
Value Environment::get_at(int distance, const Token &name)
{
    return ancestor(distance)->values[name.lexeme];
}

void Environment::assign(const Token &name, const Value &val)
{
    auto it = values.find(name.lexeme);
    if (it != values.end()) {
        it->second = val;
    } else if (outer != nullptr) {
        outer->assign(name, val);
    } else {
        error(name, "Undefined variable.");
        throw RuntimeError{};
    }
}

// 调用此函数应保证对应变量已定义
void Environment::assign_at(int distance, const Token &name, const Value &val)
{
    ancestor(distance)->values[name.lexeme] = val;
}
