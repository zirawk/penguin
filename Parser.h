#ifndef PARSER_H
#define PARSER_H

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>
#include "Expr.h"
#include "Stmt.h"
#include "Token.h"

struct ParseError {};

class Parser {
    using enum TokenType;
private:
    const std::vector<Token> tokens;
    int current;
    int loop_depth;

    bool at_end();
    const Token &peek();
    const Token &previous();
    const Token &advance();
    bool match(std::initializer_list<TokenType> types);
    const Token &consume(TokenType type, std::string msg);

    // Handle expressions
    std::unique_ptr<Expr> expression();
    std::unique_ptr<Expr> assignment();
    std::unique_ptr<Expr> logic_or();
    std::unique_ptr<Expr> logic_and();
    std::unique_ptr<Expr> equality();
    std::unique_ptr<Expr> comparison();
    std::unique_ptr<Expr> term();
    std::unique_ptr<Expr> factor();
    std::unique_ptr<Expr> unary();
    std::unique_ptr<Expr> call();
    std::unique_ptr<Expr> finish_call(std::unique_ptr<Expr> callee);
    std::unique_ptr<Expr> primary();

    // Handle statements
    std::unique_ptr<Stmt> declaration();
    std::unique_ptr<VarDeclStmt> variable_declaration();
    std::unique_ptr<FuncDeclStmt> function_declaration(std::string kind);
    std::unique_ptr<Stmt> statement();
    std::vector<std::unique_ptr<Stmt>> block();
    std::unique_ptr<ExprStmt> expression_statement();
    std::unique_ptr<BlockStmt> block_statement();
    std::unique_ptr<IfStmt> if_statement();
    std::unique_ptr<WhileStmt> while_statement();
    std::unique_ptr<BreakStmt> break_statement();
    std::unique_ptr<ContinueStmt> continue_statement();
    std::unique_ptr<ReturnStmt> return_statement();
    std::unique_ptr<ForStmt> for_statement();
    std::unique_ptr<PrintStmt> print_statement();

    void synchronize();
public:
    Parser(std::vector<Token> tokens);

    // 尽管目前所有 error() 的位置都会 throw RuntimeError
    // 仍不建议在 has_error == true 的情况下执行 parse() 产生的 AST
    std::vector<std::unique_ptr<Stmt>> parse();
};

#endif // PARSER_H
