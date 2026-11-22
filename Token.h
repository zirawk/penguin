#ifndef TOKEN_H
#define TOKEN_H

#include <string>
#include <variant>

enum class TokenType {
    // Single-character tokens.
    LEFT_PAREN, RIGHT_PAREN, LEFT_BRACE, RIGHT_BRACE,
    COMMA, DOT, MINUS, PLUS, SEMICOLON, SLASH, STAR,

    // One or two character tokens.
    BANG, BANG_EQUAL,
    EQUAL, EQUAL_EQUAL,
    GREATER, GREATER_EQUAL,
    LESS, LESS_EQUAL,

    // Literals.
    IDENTIFIER, STRING, NUMBER,

    // Keywords.
    AND, CLASS, ELSE, FALSE, FUN, FOR, IF, NIL, OR,
    PRINT, RETURN, SUPER, THIS, TRUE, VAR, WHILE,

    END_OF_FILE
};

// Literal Value
using LitVal = std::variant<std::monostate, bool, double, std::string>;

struct Token {
    TokenType type{};
    std::string lexeme;
    LitVal lit{};
    int line{};

    std::string str() const;
};

std::string stringify(const LitVal &lit);

#endif // TOKEN_H
