#include "Parser.h"
#include "Expr.h"
#include "Token.h"
#include "base.h"
#include <initializer_list>
#include <memory>
#include <string>
#include <variant>
#include <vector>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current(0) {}

std::unique_ptr<Expr> Parser::parse()
{
    try {
        auto expr = expression();
        // return局部变量，视为xvalue，默认move
        // 但编译器也可以选择NVRO
        if (at_end()) return expr;
        error(peek(), "Expect end of expression.");
        return nullptr;
    } catch (const ParseError &) {
        return nullptr;
    }
}

bool Parser::at_end()
{
    return peek().type == END_OF_FILE;
}

const Token &Parser::peek()
{
    return tokens[current];
}

const Token &Parser::previous()
{
    // TODO 该函数被调用时，current有可能为0吗？
    return tokens[current - 1];
}

const Token &Parser::advance()
{
    if (!at_end()) ++current;
    return previous();
}

bool Parser::match(std::initializer_list<TokenType> types)
{
    for (auto t : types) {
        if (peek().type == t) {
            // 用advance()的好处是，即使types里含END_OF_FILE，current也不会越界
            advance();
            return true;
        }
    }
    return false;
}

void Parser::consume(TokenType type, std::string msg)
{
    if (peek().type == type) {
        advance();
        return;
    }
    error(peek(), msg);
    throw ParseError{};
}

std::unique_ptr<Expr> Parser::expression()
{
    return equality();
}

std::unique_ptr<Expr> Parser::equality()
{
    auto expr = comparison();
    while (match({BANG_EQUAL, EQUAL_EQUAL})) {
        Token op = previous();
        auto right = comparison();
        // 等号右侧为prvalue，移动构造，下同
        expr = std::make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison()
{
    auto expr = term();
    while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
        Token op = previous();
        auto right = term();
        expr = std::make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term()
{
    auto expr = factor();
    while (match({PLUS, MINUS})) {
        Token op = previous();
        auto right = factor();
        expr = std::make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor()
{
    auto expr = unary();
    while (match({STAR, SLASH})) {
        Token op = previous();
        auto right = unary();
        expr = std::make_unique<Binary>(std::move(expr), std::move(op), std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary()
{
    if (match({BANG, MINUS})) {
        auto op = previous();
        // 不要对unique_ptr进行std::move，以保证拷贝消除
        return std::make_unique<Unary>(std::move(op), unary());
    }
    return primary();
}

std::unique_ptr<Expr> Parser::primary()
{
    if (match({FALSE})) return std::make_unique<Literal>(false);
    if (match({TRUE})) return std::make_unique<Literal>(true);
    if (match({NIL})) return std::make_unique<Literal>(std::monostate{});

    if (match({NUMBER, STRING})) return std::make_unique<Literal>(previous().lit);

    if (match({LEFT_PAREN})) {
        auto expr = expression();
        consume(RIGHT_PAREN, "Expect ')' after expression.\n");
        return std::make_unique<Grouping>(std::move(expr));
    }
    // Control flow shouldn't reach here
    error(peek(), "Expected expression.");
    throw ParseError{};
}

void Parser::synchronize()
{
    advance();

    while (!at_end()) {
        if (previous().type == SEMICOLON) return;

        switch (peek().type) {
        case CLASS: case FUN:    case FOR: case IF:
        case PRINT: case RETURN: case VAR: case WHILE:
            return;
        default:
            break;
        }

        advance();
    }
}
