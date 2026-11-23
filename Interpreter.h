#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Expr.h"
#include "Token.h"
#include "value.h"

class Interpreter : public ExprVisitor {
    using enum TokenType;
private:
    Value result;

    static double number(const Value &v, const Token &op);
    static Value add(const Value &l, const Value &r, const Token &op);
    virtual void visit(const Binary &o) override;
    virtual void visit(const Unary &o) override;
    virtual void visit(const Grouping &o) override;
    virtual void visit(const Literal &o) override;
public:
    Value interpret(const Expr &expr);
};

#endif // INTERPRETER_H
