#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "Expr.h"
#include <string>

class AstPrinter : public ExprVisitor {
private:
    std::string result;
public:
    std::string print(const Expr &expr);
    virtual void visit(const Binary &o) override;
    virtual void visit(const Unary &o) override;
    virtual void visit(const Grouping &o) override;
    virtual void visit(const Literal &o) override;
};

#endif // AST_PRINTER_H
