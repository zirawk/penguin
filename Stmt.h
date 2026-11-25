#ifndef STMT_H
#define STMT_H

#include "Expr.h"
#include <memory>
#include <utility>
#include <vector>

struct ExprStmt;
struct VarDeclStmt;
struct BlockStmt;
struct IfStmt;
struct WhileStmt;
struct ForStmt;
struct BreakStmt;
struct ContinueStmt;
struct PrintStmt;

class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visit(const ExprStmt &o) = 0;
    virtual void visit(const VarDeclStmt &o) = 0;
    virtual void visit(const BlockStmt &o) = 0;
    virtual void visit(const IfStmt &o) = 0;
    virtual void visit(const WhileStmt &o) = 0;
    virtual void visit(const ForStmt &o) = 0;
    virtual void visit(const BreakStmt &o) = 0;
    virtual void visit(const ContinueStmt &o) = 0;
    virtual void visit(const PrintStmt &o) = 0;
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor &v) const = 0;
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;

    ExprStmt(std::unique_ptr<Expr> expr_) : expr(std::move(expr_)) {}

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

struct VarDeclStmt : Stmt {
    Token name;
    std::unique_ptr<Expr> initializer;

    VarDeclStmt(Token name_, std::unique_ptr<Expr> initializer_)
        : name(std::move(name_)), initializer(std::move(initializer_)) {}

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> stmts;

    BlockStmt(std::vector<std::unique_ptr<Stmt>> stmts_)
        : stmts(std::move(stmts_)) {}

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> then_stmt;
    std::unique_ptr<Stmt> else_stmt;

    IfStmt(std::unique_ptr<Expr> condition_, std::unique_ptr<Stmt> then_stmt_, std::unique_ptr<Stmt> else_stmt_)
        : condition(std::move(condition_)), then_stmt(std::move(then_stmt_)), else_stmt(std::move(else_stmt_)) {}

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;

    WhileStmt(std::unique_ptr<Expr> condition_, std::unique_ptr<Stmt> body_)
        : condition(std::move(condition_)), body(std::move(body_)) {}

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

struct ForStmt : Stmt {
    std::unique_ptr<Stmt> initializer;
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> increment;
    std::unique_ptr<Stmt> body;

    ForStmt(std::unique_ptr<Stmt> initializer_, std::unique_ptr<Expr> condition_,
            std::unique_ptr<Expr> increment_, std::unique_ptr<Stmt> body_)
        : initializer(std::move(initializer_)), condition(std::move(condition_)),
          increment(std::move(increment_)), body(std::move(body_)) {}

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this);}
};

struct BreakStmt : Stmt {
    BreakStmt() = default;

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

struct ContinueStmt : Stmt {
    ContinueStmt() = default;

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

struct PrintStmt : Stmt {
    std::unique_ptr<Expr> expr;

    PrintStmt(std::unique_ptr<Expr> expr_) : expr(std::move(expr_)) {}

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

#endif // STMT_H
