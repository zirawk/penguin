#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "value.h"
#include "Token.h"
#include <string>
#include <unordered_map>

class Environment {
private:
    std::unordered_map<std::string, Value> values;
public:
    void define(const std::string &name, const Value &val);
    Value get(const Token &name) const;
    void assign(const Token &name, const Value &val);
};

#endif // ENVIRONMENT_H
