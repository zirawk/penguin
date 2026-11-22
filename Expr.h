#ifndef EXPR_H
#define EXPR_H

#include "Token.h"
#include <memory>

struct Binary;
struct Unary;
struct Grouping;
struct Literal;

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual void visit(const Binary &o) = 0;
    virtual void visit(const Unary &o) = 0;
    virtual void visit(const Grouping &o) = 0;
    virtual void visit(const Literal &o) = 0;
};

struct Expr {
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor &v) const = 0;
};

struct Binary : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    Binary(std::unique_ptr<Expr> l, Token o, std::unique_ptr<Expr> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Unary : Expr {
    Token op;
    std::unique_ptr<Expr> right;

    Unary(Token o, std::unique_ptr<Expr> r)
        : op(std::move(o)), right(std::move(r)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Grouping : Expr {
    std::unique_ptr<Expr> expr;

    Grouping(std::unique_ptr<Expr> exp) : expr(std::move(exp)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Literal : Expr {
    LitVal lit;

    Literal(LitVal l) : lit(std::move(l)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

#endif // EXPR_H
