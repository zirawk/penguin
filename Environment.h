#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "value.h"
#include "Token.h"
#include <memory>
#include <string>
#include <unordered_map>

class Environment {
private:
    std::shared_ptr<Environment> outer;
    std::unordered_map<std::string, Value> values;
public:
    Environment();
    Environment(std::shared_ptr<Environment> out);

    void define(const std::string &name, const Value &val);
    Value get(const Token &name) const;
    void assign(const Token &name, const Value &val);
};

#endif // ENVIRONMENT_H
