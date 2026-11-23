#include "Parser.h"
#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include "error.h"
#include <initializer_list>
#include <memory>
#include <string>
#include <variant>
#include <vector>

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current(0) {}

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

const Token &Parser::consume(TokenType type, std::string msg)
{
    if (peek().type == type) {
        advance();
        return previous();
    }
    error(peek(), msg);
    throw ParseError{};
}

std::unique_ptr<Expr> Parser::expression()
{
    return assignment();
}

std::unique_ptr<Expr> Parser::assignment()
{
    auto expr = equality();

    // 处理赋值表达式
    if (match({EQUAL})) {
        const Token &equal = previous(); // for error()
        auto value = assignment(); // 右结合
        if (auto var = dynamic_cast<Variable *>(expr.get())) {
            Token name = std::move(var->name);
            return std::make_unique<Assign>(name, std::move(value));
        }

        error(equal, "Invalid assignment target.");
        throw ParseError{};
    }

    return expr;
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

    if (match({NUMBER, STRING})) return std::make_unique<Literal>(previous().literal);

    if (match({IDENTIFIER})) return std::make_unique<Variable>(previous());

    if (match({LEFT_PAREN})) {
        auto expr = expression();
        consume(RIGHT_PAREN, "Expect ')' after expression.");
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

std::unique_ptr<Stmt> Parser::declaration()
{
    try {
        if (match({VAR})) return variable_declaration();
        return statement();
    } catch (const ParseError &) {
        // 语句作为恢复单位，在此 sync
        synchronize();
        return nullptr;
    }
}

std::unique_ptr<VarDeclStmt> Parser::variable_declaration()
{
    const Token &name = consume(IDENTIFIER, "Expect variable name.");

    // 先置 nil，若有初始化表达式再更新
    std::unique_ptr<Expr> initializer = std::make_unique<Literal>(std::monostate{});
    if (match({EQUAL})) initializer = expression();

    consume(SEMICOLON, "Expect ';' after variable declaration.");
    // std::move(name) 不会起到任何作用，name 依然会被拷贝
    // 原因见 std::move() 实现细节
    return std::make_unique<VarDeclStmt>(name, std::move(initializer));
}

std::unique_ptr<Stmt> Parser::statement()
{
    if (match({PRINT})) return print_statement();
    return expression_statement();
}

std::unique_ptr<ExprStmt> Parser::expression_statement()
{
    auto expr = expression();
    consume(SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<PrintStmt> Parser::print_statement()
{
    auto value = expression();
    consume(SEMICOLON, "Expect ';' after value.");
    return std::make_unique<PrintStmt>(std::move(value));
}

std::vector<std::unique_ptr<Stmt>> Parser::parse()
{
    std::vector<std::unique_ptr<Stmt>> stmts;
    while (!at_end()) {
        if (auto stmt = declaration())
            stmts.push_back(std::move(stmt));
    }
    return stmts;
}
