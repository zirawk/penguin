#include "AstPrinter.h"
#include "value.h"

void AstPrinter::visit(const Binary &o)
{
    o.left->accept(*this);
    auto s1 = result;
    o.right->accept(*this);
    auto s2 = result;
    result = '(' + o.op.lexeme + ' ' + s1 + ' ' + s2 + ')';
}
void AstPrinter::visit(const Unary &o)
{
    o.right->accept(*this);
    result = '(' + o.op.lexeme + ' ' + result + ')';
}

void AstPrinter::visit(const Grouping &o)
{
    o.expr->accept(*this);
    result = "(group " + result + ')';
}

void AstPrinter::visit(const Literal &o)
{
    result = stringify(o.lit);
}

std::string AstPrinter::print(const Expr &expr)
{
    // 同一个AstPrinter可能被多次使用，需清空
    result.clear();
    expr.accept(*this);
    return result;
}

