#include "Token.h"

std::string Token::str() const
{
    return std::to_string(line) + " | " + std::to_string(static_cast<int>(type))
        + " | " + lexeme + " | " + stringify(literal);
}
