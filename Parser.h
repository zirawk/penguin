#ifndef PARSER_H
#define PARSER_H

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>
#include "Expr.h"
#include "Token.h"

struct ParseError {};

class Parser {
    using enum TokenType;
private:
    const std::vector<Token> tokens;
    int current;

    bool at_end();
    const Token &peek();
    const Token &previous();
    const Token &advance();
    bool match(std::initializer_list<TokenType> types);
    void consume(TokenType type, std::string msg);

    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> primary();

    void synchronize();
public:
    Parser(std::vector<Token> tokens);
    std::unique_ptr<Expr> parse();
};

#endif // PARSER_H
