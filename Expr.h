#ifndef EXPR_H
#define EXPR_H

#include "Token.h"
#include <memory>
#include <vector>

struct Assign;
struct Logical;
struct Binary;
struct Unary;
struct Call;
struct Grouping;
struct Literal;
struct Variable;

class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;
    virtual void visit(const Assign &o) = 0;
    virtual void visit(const Logical &o) = 0;
    virtual void visit(const Binary &o) = 0;
    virtual void visit(const Unary &o) = 0;
    virtual void visit(const Call &o) = 0;
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

    Assign(Token name_, std::unique_ptr<Expr> value_)
        : name(std::move(name_)), value(std::move(value_)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Logical : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    Logical(std::unique_ptr<Expr> left_, Token op_, std::unique_ptr<Expr> right_)
        : left(std::move(left_)), op(std::move(op_)), right(std::move(right_)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Binary : Expr {
    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;

    Binary(std::unique_ptr<Expr> left_, Token op_, std::unique_ptr<Expr> right_)
        : left(std::move(left_)), op(std::move(op_)), right(std::move(right_)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Unary : Expr {
    Token op;
    std::unique_ptr<Expr> right;

    Unary(Token op_, std::unique_ptr<Expr> right_)
        : op(std::move(op_)), right(std::move(right_)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Call : Expr {
    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> args;
    Token rparen;

    Call(std::unique_ptr<Expr> callee_, std::vector<std::unique_ptr<Expr>> args_, Token rparen_)
        : callee(std::move(callee_)), args(std::move(args_)), rparen(std::move(rparen_)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Grouping : Expr {
    std::unique_ptr<Expr> expr;

    Grouping(std::unique_ptr<Expr> expr_) : expr(std::move(expr_)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Literal : Expr {
    Value lit;

    Literal(Value lit_) : lit(std::move(lit_)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

struct Variable : Expr {
    Token name;

    Variable(Token name_) : name(std::move(name_)) {}

    virtual void accept(ExprVisitor &v) const override
    { v.visit(*this); }
};

#endif // EXPR_H
