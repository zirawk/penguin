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

namespace {
class LoopGuard {
private:
    int &slot;
public:
    LoopGuard(int &slot_) : slot(slot_)
    { ++slot; }

    LoopGuard(const LoopGuard &) = delete;
    LoopGuard &operator=(const LoopGuard &) = delete;

    ~LoopGuard() { --slot; }
};
}

Parser::Parser(std::vector<Token> tokens)
    : tokens(std::move(tokens)), current(0), loop_depth(0) {}

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
    auto expr = logic_or();

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

std::unique_ptr<Expr> Parser::logic_or()
{
    auto expr = logic_and();
    while (match({OR})) {
        const Token &op = previous();
        auto right = logic_and();
        expr = std::make_unique<Logical>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::logic_and()
{
    auto expr = equality();
    while (match({AND})) {
        const Token &op = previous();
        auto right = equality();
        expr = std::make_unique<Logical>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::equality()
{
    auto expr = comparison();
    while (match({BANG_EQUAL, EQUAL_EQUAL})) {
        const Token &op = previous();
        auto right = comparison();
        // 等号右侧为prvalue，移动构造，下同
        expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::comparison()
{
    auto expr = term();
    while (match({GREATER, GREATER_EQUAL, LESS, LESS_EQUAL})) {
        const Token &op = previous();
        auto right = term();
        expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::term()
{
    auto expr = factor();
    while (match({PLUS, MINUS})) {
        const Token &op = previous();
        auto right = factor();
        expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::factor()
{
    auto expr = unary();
    while (match({STAR, SLASH})) {
        const Token &op = previous();
        auto right = unary();
        expr = std::make_unique<Binary>(std::move(expr), op, std::move(right));
    }
    return expr;
}

std::unique_ptr<Expr> Parser::unary()
{
    if (match({BANG, MINUS})) {
        const Token &op = previous();
        // 不要对unique_ptr进行std::move，以保证拷贝消除
        return std::make_unique<Unary>(op, unary());
    }
    return call();
}

std::unique_ptr<Expr> Parser::call()
{
    auto expr = primary();

    while (true) {
        if (match({LEFT_PAREN})) {
            // expr 求值结果为函数，生成对应的 Call
            expr = finish_call(std::move(expr));
        } else {
            break;
        }
    }

    return expr;
}

std::unique_ptr<Expr> Parser::finish_call(std::unique_ptr<Expr> callee)
{
    std::vector<std::unique_ptr<Expr>> args;
    if (peek().type != RIGHT_PAREN) {
        do {
            if (args.size() >= 255) {
                error(previous(), "Expect no more than 255 arguments.");
                // TODO throw ParseError{};
            }
            args.push_back(expression());
        } while (match({COMMA}));
    }
    const Token &rparen = consume(RIGHT_PAREN, "Expect ')' after arguments.");

    return std::make_unique<Call>(std::move(callee), std::move(args), rparen);
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
    error(peek(), "Expect expression.");
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
        if (match({FUN})) return function_declaration("function");
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

std::unique_ptr<FuncDeclStmt> Parser::function_declaration(std::string kind)
{
    const Token &name = consume(IDENTIFIER, "Expect " + kind + " name.");
    consume(LEFT_PAREN, "Expect '(' after " + kind + " name.");

    std::vector<Token> params;
    // 填充形参名
    if (peek().type != RIGHT_PAREN) {
        do {
            if (params.size() >= 255) {
                error(peek(), "Expect no more than 255 parameters.");
            }
            params.push_back(consume(IDENTIFIER, "Expect parameter name."));
        } while (match({COMMA}));
    }
    consume(RIGHT_PAREN, "Expect ')' after parameters.");

    consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
    auto body = block();

    return std::make_unique<FuncDeclStmt>(name, std::move(params), std::move(body));
}

std::unique_ptr<Stmt> Parser::statement()
{
    if (match({PRINT})) return print_statement();
    if (match({LEFT_BRACE})) return block_statement();
    if (match({IF})) return if_statement();
    if (match({WHILE})) return while_statement();
    if (match({FOR})) return for_statement();
    if (match({BREAK})) return break_statement();
    if (match({CONTINUE})) return continue_statement();
    if (match({RETURN})) return return_statement();
    return expression_statement();
}

std::vector<std::unique_ptr<Stmt>> Parser::block()
{
    std::vector<std::unique_ptr<Stmt>> stmts;
    // 为什么不 !match({RIGHT_BRACE}) && !at_end()
    // 若这样做，就不能直接 consume 而要取 previous() 来判断是哪种情形
    while (peek().type != RIGHT_BRACE && !at_end()) {
        stmts.push_back(declaration());
    }
    consume(RIGHT_BRACE, "Expect '}' after block.");
    return stmts;
}

std::unique_ptr<ExprStmt> Parser::expression_statement()
{
    auto expr = expression();
    consume(SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ExprStmt>(std::move(expr));
}

std::unique_ptr<BlockStmt> Parser::block_statement()
{
    return std::make_unique<BlockStmt>(block());
}

std::unique_ptr<IfStmt> Parser::if_statement()
{
    consume(LEFT_PAREN, "Expect '(' after 'if'.");
    auto cond = expression();
    consume(RIGHT_PAREN, "Expect ')' after 'if' condition.");
    auto then_stmt = statement();
    std::unique_ptr<Stmt> else_stmt = nullptr;
    if (match({ELSE})) else_stmt = statement();
    return std::make_unique<IfStmt>(std::move(cond), std::move(then_stmt), std::move(else_stmt));
}

std::unique_ptr<WhileStmt> Parser::while_statement()
{
    LoopGuard guard(loop_depth);

    consume(LEFT_PAREN, "Expect '(' after 'while'.");
    auto cond = expression();
    consume(RIGHT_PAREN, "Expect ')' after 'while' condition.");
    auto body = statement();

    return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
}

std::unique_ptr<ForStmt> Parser::for_statement()
{
    LoopGuard guard(loop_depth);

    consume(LEFT_PAREN, "Expect '(' after 'for'.");

    std::unique_ptr<Stmt> init;
    if (match({SEMICOLON})) init = nullptr;
    else if (match({VAR})) init = variable_declaration();
    else init = expression_statement();

    std::unique_ptr<Expr> cond;
    if (peek().type != SEMICOLON) cond = expression();
    else cond = std::make_unique<Literal>(true);
    consume(SEMICOLON, "Expect ';' after 'for' condition.");

    std::unique_ptr<Expr> incr = nullptr;
    if (peek().type != RIGHT_PAREN) incr = expression();

    consume(RIGHT_PAREN, "Expect ')' after 'for' clauses.");

    auto body = statement();

    return std::make_unique<ForStmt>(std::move(init), std::move(cond), std::move(incr), std::move(body));
}

std::unique_ptr<BreakStmt> Parser::break_statement()
{
    if (loop_depth == 0) {
        error(previous(), "'break' not in loop.");
        throw ParseError{};
    }
    consume(SEMICOLON, "Expect ';' after 'break'.");
    return std::make_unique<BreakStmt>();
}

std::unique_ptr<ContinueStmt> Parser::continue_statement()
{
    if (loop_depth == 0) {
        error(previous(), "'continue' not in loop.");
        throw ParseError{};
    }
    consume(SEMICOLON, "Expect ';' after 'continue'.");
    return std::make_unique<ContinueStmt>();
}

std::unique_ptr<ReturnStmt> Parser::return_statement()
{
    const Token &token = previous();
    std::unique_ptr<Expr> value = nullptr;
    if (peek().type != SEMICOLON) value = expression();
    consume(SEMICOLON, "Expect ';' after return value.");
    return std::make_unique<ReturnStmt>(token, std::move(value));
}

// Desugar
// std::unique_ptr<Stmt> Parser::for_statement()
// {
//     LoopGuard guard(loop_depth);
// 
//     consume(LEFT_PAREN, "Expect '(' after 'for'.");
// 
//     std::unique_ptr<Stmt> init;
//     if (match({SEMICOLON})) init = nullptr;
//     else if (match({VAR})) init = variable_declaration();
//     else init = expression_statement();
// 
//     std::unique_ptr<Expr> cond = std::make_unique<Literal>(true);
//     if (peek().type != SEMICOLON) cond = expression();
//     consume(SEMICOLON, "Expect ';' after 'for' condition.");
// 
//     std::unique_ptr<Expr> incr = nullptr;
//     if (peek().type != RIGHT_PAREN) incr = expression();
//     consume(RIGHT_PAREN, "Expect ';' after 'for' clauses.");
// 
//     // body - for 循环体
//     auto body = statement();
// 
//     if (incr) {
//         std::vector<std::unique_ptr<Stmt>> while_body;
//         while_body.push_back(std::move(body));
//         while_body.push_back(std::make_unique<ExprStmt>(std::move(incr)));
//         // body - for 循环体 + 增量表达式 组成的块
//         body = std::make_unique<BlockStmt>(std::move(while_body));
//     }
// 
//     // body - while 语句
//     body = std::make_unique<WhileStmt>(std::move(cond), std::move(body));
// 
//     if (init) {
//         std::vector<std::unique_ptr<Stmt>> whole_body;
//         whole_body.push_back(std::move(init));
//         whole_body.push_back(std::move(body));
//         // body - 初始化语句 + while 语句 组成的块
//         body = std::make_unique<BlockStmt>(std::move(whole_body));
//     }
// 
//     return body;
// }

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
