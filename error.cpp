#include "error.h"
#include "Token.h"
#include <iostream>
#include <string_view>

bool has_error = false;

namespace {
void report(int line, std::string_view where, std::string_view message)
{
    std::cerr << "[line " << line << "] Error" << where << ": " << message << '\n';
    has_error = true;
}
}

void error(int line, std::string_view message)
{
    report(line, "", message);
    has_error = true;
}

void error(const Token &token, std::string_view message)
{
    if (token.type == TokenType::END_OF_FILE) {
        report(token.line, " at end", message);
    } else {
        report(token.line, " at '" + token.lexeme + "'", message);
    }
}

