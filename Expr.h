#ifndef EXPR_H
#define EXPR_H

#include "Token.h"
#include <memory>

struct Assign;
struct Binary;
struct Unary;
struct Grouping;
struct Literal;
struct Variable;

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual void visit(const Assign &o) = 0;
    virtual void visit(const Binary &o) = 0;
    virtual void visit(const Unary &o) = 0;
    virtual void visit(const Grouping &o) = 0;
    virtual void visit(const Literal &o) = 0;
    virtual void visit(const Variable &o) = 0;
};

struct Expr {
    virtual ~Expr() = default;
    virtual void accept(ExprVisitor &v) const = 0;
};

struct Assign : Expr {
    Token name;
    std::unique_ptr<Expr> value;

    Assign(Token nm, std::unique_ptr<Expr> val)
        : name(std::move(nm)), value(std::move(val)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
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
    Value lit;

    Literal(Value l) : lit(std::move(l)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Variable : Expr {
    Token name;

    Variable(Token nm) : name(std::move(nm)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

#endif // EXPR_H
