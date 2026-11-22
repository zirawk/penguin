#include "value.h"
#include <charconv>
#include <cmath>
#include <string>
#include <system_error>

namespace{
std::string number_to_string(double v)
{
    if (std::isnan(v)) return "nan";
    if (std::isinf(v)) return v < 0 ? "-inf" : "inf";

    char buf[64];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), v);
    if (ec != std::errc{}) return std::to_string(v);
    return std::string(buf, ptr);
}
}

// Value -> std::string
std::string stringify(const Value &v)
{
    if (auto *b = std::get_if<bool>(&v))
        return *b ? "true" : "false";
    if (auto *d = std::get_if<double>(&v))
        return number_to_string(*d);
    if (auto *s = std::get_if<std::string>(&v))
        return *s;
    return "nil";
}
