#ifndef VALUE_H
#define VALUE_H

#include <memory>
#include <variant>
#include <string>
#include <vector>

class Interpreter;
class Callable;

using Value = std::variant<std::monostate, bool, double, std::string,
                           std::shared_ptr<Callable>>;

class Callable {
public:
    virtual ~Callable() = default;
    virtual unsigned arity() = 0;
    virtual Value call(Interpreter &interp, const std::vector<Value> &args) = 0;
    virtual std::string str() = 0;
};

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
