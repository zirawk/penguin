#include "Token.h"
#include <charconv>
#include <cmath>
#include <string>
#include <system_error>
#include <variant>

std::string number_to_string(double v)
{
    if (std::isnan(v)) return "nan";
    if (std::isinf(v)) return v < 0 ? "-inf" : "inf";

    char buf[64];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), v);
    if (ec != std::errc{}) return std::to_string(v);
    return std::string(buf, ptr);
}

// LitVal -> std::string
std::string stringify(const LitVal &lit)
{
    if (auto *b = std::get_if<bool>(&lit))
        return *b ? "true" : "false";
    if (auto *d = std::get_if<double>(&lit))
        return number_to_string(*d);
    if (auto *s = std::get_if<std::string>(&lit))
        return *s;
    return "nil";
}

std::string Token::str() const
{
    return std::to_string(line) + " | " + std::to_string(static_cast<int>(type))
        + " | " + lexeme + " | " + stringify(lit);
}
