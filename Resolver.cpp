#include "error.h"
#include "Resolver.h"
#include "Interpreter.h"
#include <memory>

Resolver::Resolver(Interpreter &interp)
    : interpreter(interp), current_func(NONE), in_loop(false) {}

void Resolver::resolve(const std::unique_ptr<Expr> &expr)
{
    expr->accept(*this);
}

void Resolver::resolve(const std::unique_ptr<Stmt> &stmt)
{
    stmt->accept(*this);
}

void Resolver::resolve(const std::vector<std::unique_ptr<Stmt>> &stmts)
{
    for (const auto &stmt : stmts) resolve(stmt);
}

void Resolver::begin_scope()
{
    scopes.emplace_back();
}

void Resolver::end_scope()
{
    scopes.pop_back();
}

void Resolver::define(const Token &name)
{
    if (scopes.empty()) return;
    scopes[scopes.size() - 1].insert(name.lexeme);
}

void Resolver::resolve_local(const Expr &expr, const Token &name)
{
    for (int i = scopes.size() - 1; i >= 0; --i) {
        if (scopes[i].contains(name.lexeme)) {
            interpreter.resolve(expr, scopes.size() - 1 - i);
            return;
        }
    }
}

void Resolver::resolve_function(const FuncDeclStmt &func, FuncType type)
{
    FuncType outer_func = current_func;
    current_func = type;
    // break/continue 不可跨函数抛出
    bool outer_loop = in_loop;
    in_loop = false;

    begin_scope();
    for (const Token &param : func.params) define(param);
    resolve(func.body);
    end_scope();

    in_loop = outer_loop;
    current_func = outer_func;
}

void Resolver::visit(const Assign &o)
{
    resolve(o.value);
    resolve_local(o, o.name);
}

void Resolver::visit(const Logical &o)
{
    resolve(o.left);
    resolve(o.right);
}

void Resolver::visit(const Binary &o)
{
    resolve(o.left);
    resolve(o.right);
}

void Resolver::visit(const Unary &o)
{
    resolve(o.right);
}

void Resolver::visit(const Call &o)
{
    resolve(o.callee);
    for (const auto &arg : o.args) resolve(arg);
}

void Resolver::visit(const Grouping &o)
{
    resolve(o.expr);
}

void Resolver::visit(const Literal &) {}

void Resolver::visit(const Variable &o)
{
    resolve_local(o, o.name);
}

void Resolver::visit(const ExprStmt &o)
{
    resolve(o.expr);
}

void Resolver::visit(const VarDeclStmt &o)
{
    if (o.initializer) resolve(o.initializer);
    define(o.name);
}

void Resolver::visit(const FuncDeclStmt &o)
{
    define(o.name);
    resolve_function(o, FUNCTION);
}

void Resolver::visit(const BlockStmt &o)
{
    begin_scope();
    resolve(o.stmts);
    end_scope();
}

void Resolver::visit(const IfStmt &o)
{
    resolve(o.condition);
    resolve(o.then_stmt);
    if (o.else_stmt) resolve(o.else_stmt);
}

void Resolver::visit(const WhileStmt &o)
{
    bool prev = in_loop;
    in_loop = true;

    resolve(o.condition);
    resolve(o.body);

    in_loop = prev;
}

void Resolver::visit(const ForStmt &o)
{
    bool prev = in_loop;
    in_loop = true;

    begin_scope();
    if (o.initializer) resolve(o.initializer);
    resolve(o.condition);
    if (o.increment) resolve(o.increment);
    resolve(o.body);
    end_scope();

    in_loop = prev;
}

void Resolver::visit(const BreakStmt &o)
{
    if (!in_loop) error(o.keyword, "'break' not in loop.");
}

void Resolver::visit(const ContinueStmt &o)
{
    if (!in_loop) error(o.keyword, "'continue' not in loop.");
}

void Resolver::visit(const ReturnStmt &o)
{
    if (current_func == NONE) {
        error(o.keyword, "'return' not in function.");
    }
    if (o.value) resolve(o.value);
}

void Resolver::visit(const PrintStmt &o)
{
    resolve(o.expr);
}

