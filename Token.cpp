#include "Token.h"
#include <string>
#include <variant>

// Literal -> std::string
static std::string stringify(const Literal &lit)
{
    if (auto *d = std::get_if<double>(&lit))
        return std::to_string(*d);
    if (auto *s = std::get_if<std::string>(&lit))
        return *s;
    return "nil";
}

std::string Token::str() const
{
    return std::to_string(line) + " | " + std::to_string(static_cast<int>(type))
        + " | " + lexeme + " | " + stringify(literal);
}
