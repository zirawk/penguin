#include "Interpreter.h"
#include "Token.h"
#include "value.h"
#include "error.h"
#include <iostream>
#include <memory>
#include <variant>
#include <vector>

double Interpreter::number(const Value &v, const Token &op)
{
    if (auto d = std::get_if<double>(&v)) return *d;
    error(op, "Operand must be a number.");
    throw RuntimeError{};
}

Value Interpreter::add(const Value &l, const Value &r, const Token &op)
{
    auto ld = std::get_if<double>(&l);
    auto rd = std::get_if<double>(&r);
    if (ld && rd) return *ld + *rd;
    auto ls = std::get_if<std::string>(&l);
    auto rs = std::get_if<std::string>(&r);
    if (ls && rs) return *ls + *rs;
    error(op, "Operands must be numbers or strings.");
    throw RuntimeError{};
}

// 赋值表达式 - 对表达式求值同时完成赋值
void Interpreter::visit(const Assign &o)
{
    o.value->accept(*this);
    env.assign(o.name, result);
}

void Interpreter::visit(const Binary &o)
{
    o.left->accept(*this);
    Value l = result;
    o.right->accept(*this);
    Value r = result;

    switch (o.op.type) {
    case MINUS:
        result = number(l, o.op) - number(r, o.op);
        break;
    case STAR:
        result = number(l, o.op) * number(r, o.op);
        break;
    case SLASH:
        result = number(l, o.op) / number(r, o.op);
        break;
    case GREATER:
        result = number(l, o.op) > number(r, o.op);
        break;
    case GREATER_EQUAL:
        result = number(l, o.op) >= number(r, o.op);
        break;
    case LESS:
        result = number(l, o.op) < number(r, o.op);
        break;
    case LESS_EQUAL:
        result = number(l, o.op) <= number(r, o.op);
        break;

    case BANG_EQUAL:
        result = l != r;
        break;
    case EQUAL_EQUAL:
        result = l == r;
        break;

    case PLUS:
        result = add(l, r, o.op);
        break;
    default:
        break; // Unreachable
    }
}

void Interpreter::visit(const Unary &o)
{
    // 对右表达式求值，结果在result中
    o.right->accept(*this);

    switch (o.op.type) {
    case BANG:
        result = !boolean(result);
        break;
    case MINUS:
        result = -number(result, o.op);
        break;
    default:
        break; // 理论上，这条分支不可能到达
    }
}

void Interpreter::visit(const Grouping &o)
{
    // visit后结果自然在result中
    o.expr->accept(*this);
}

void Interpreter::visit(const Literal &o)
{
    result = o.lit;
}

void Interpreter::visit(const Variable &o)
{
    result = env.get(o.name);
}

void Interpreter::visit(const ExprStmt &o)
{
    // Interpreter as ExprVisitor，作用为求值
    o.expr->accept(*this);
}

void Interpreter::visit(const VarDeclStmt &o)
{
    // 对变量的初始化表达式求值
    o.initializer->accept(*this);
    env.define(o.name.lexeme, result);
}

void Interpreter::visit(const PrintStmt &o)
{
    o.expr->accept(*this);
    std::cout << stringify(result) << '\n';
}

void Interpreter::interpret(const std::vector<std::unique_ptr<Stmt>> &stmts)
{
    try {
        // Interpreter as StmtVisitor，作用为执行语句
        for (const auto &stmt : stmts) stmt->accept(*this);
    } catch (const RuntimeError &) {
        // error() 已报告，停止执行剩余语句
    }
}
