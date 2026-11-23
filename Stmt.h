#ifndef STMT_H
#define STMT_H

#include "Expr.h"
#include <memory>
#include <utility>

struct ExprStmt;
struct VarDeclStmt;
struct PrintStmt;

class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visit(const ExprStmt &o) = 0;
    virtual void visit(const VarDeclStmt &o) = 0;
    virtual void visit(const PrintStmt &o) = 0;
};

struct Stmt {
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor &v) const = 0;
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;

    ExprStmt(std::unique_ptr<Expr> exp) : expr(std::move(exp)) {}

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

struct VarDeclStmt : Stmt {
    Token name;
    std::unique_ptr<Expr> initializer;

    VarDeclStmt(Token nm, std::unique_ptr<Expr> init)
        : name(std::move(nm)), initializer(std::move(init)) {}

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

struct PrintStmt : Stmt {
    std::unique_ptr<Expr> expr;

    PrintStmt(std::unique_ptr<Expr> exp) : expr(std::move(exp)) {}

    virtual void accept(StmtVisitor &v) const override
    { v.visit(*this); }
};

#endif // STMT_H
