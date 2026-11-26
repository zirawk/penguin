#include "Interpreter.h"
#include "Environment.h"
#include "Stmt.h"
#include "Token.h"
#include "Func.h"
#include "value.h"
#include "error.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>

namespace {
class EnvGuard {
private:
    std::shared_ptr<Environment> &slot;
    std::shared_ptr<Environment> saved;
public:
    // 初始化引用，保存原始值
    EnvGuard(std::shared_ptr<Environment> &slot_, std::shared_ptr<Environment> next)
        : slot(slot_), saved(slot_)
    { slot = std::move(next); } // 赋新值
    
    EnvGuard(const EnvGuard &) = delete;
    EnvGuard &operator=(const EnvGuard &) = delete;

    // 析构时恢复原始值
    ~EnvGuard()
    { slot = std::move(saved); }
};

struct Break {};
struct Continue {};

class ClockFunc : public Callable {
public:
    virtual unsigned arity() override
    { return 0; }

    virtual Value call(Interpreter &, const std::vector<Value> &) override
    {
        using namespace std::chrono;
        return duration<double>(steady_clock::now().time_since_epoch()).count();
    }

    virtual std::string str() override
    { return "<native fun clock>"; }
};
}

Interpreter::Interpreter() : env(std::make_shared<Environment>()), global(env)
{
    global->define("clock", std::make_shared<ClockFunc>());
}

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

Value Interpreter::evaluate(const Expr &expr)
{
    // Interpreter as ExprVisitor，作用为求值
    expr.accept(*this);
    return std::move(result);
}

void Interpreter::execute(const Stmt &stmt)
{
    // Interpreter as StmtVisitor，作用为执行语句
    stmt.accept(*this);
}

// 赋值表达式 - 对表达式求值同时完成赋值
void Interpreter::visit(const Assign &o)
{
    Value val = evaluate(*o.value);
    // env->assign(o.name, val);
    auto it = locals.find(&o);
    if (it != locals.end()) {
        int distance = it->second;
        env->assign_at(distance, o.name, val);
    } else {
        global->assign(o.name, val);
    }
    // result = std::move(val);
}

// 与 Lua 相似，逻辑运算的结果未必是布尔值
void Interpreter::visit(const Logical &o)
{
    Value left = evaluate(*o.left);

    // 短路
    if (o.op.type == AND) {
        // left 为假 -> 直接返回 left
        if (!boolean(left)) {
            result = std::move(left);
            return;
        }
    } else { // OR
        // left 为真 -> 直接返回 left
        if (boolean(left)) {
            result = std::move(left);
            return;
        }
    }

    // 未短路，返回 right
    result = evaluate(*o.right);
}

void Interpreter::visit(const Binary &o)
{
    Value l = evaluate(*o.left);
    Value r = evaluate(*o.right);

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
    Value right = evaluate(*o.right);

    switch (o.op.type) {
    case BANG:
        result = !boolean(right);
        break;
    case MINUS:
        result = -number(right, o.op);
        break;
    default:
        break; // Unreachable
    }
}

void Interpreter::visit(const Call &o)
{
    Value callee = evaluate(*o.callee);
    // std::shared_ptr<Callable> *fn
    auto fn = std::get_if<std::shared_ptr<Callable>>(&callee);
    if (!fn) {
        error(o.rparen, "Calling an uncallable target.");
        throw RuntimeError{};
    }

    // shared_ptr 拷贝
    auto func = *fn;

    if (o.args.size() > func->arity()) {
        error(o.rparen, "Expect no more than " + std::to_string(func->arity())
                + " arguments but got " + std::to_string(o.args.size()) + ".");
        throw RuntimeError{};
    }

    std::vector<Value> args;
    for (const auto &arg : o.args) {
        // 对每个参数求值
        args.push_back(evaluate(*arg));
    }

    result = func->call(*this, args);
}

void Interpreter::visit(const Grouping &o)
{
    result = evaluate(*o.expr);
}

void Interpreter::visit(const Literal &o)
{
    // o.lit 有 const 属性，std::move 无效
    result = o.lit;
}

void Interpreter::visit(const Variable &o)
{
    // result = env->get(o.name);
    auto it = locals.find(&o);
    if (it != locals.end()) {
        result = env->get_at(it->second, o.name);
    } else {
        result = global->get(o.name);
    }
}

void Interpreter::visit(const ExprStmt &o)
{
    evaluate(*o.expr);
}

void Interpreter::visit(const VarDeclStmt &o)
{
    // 对变量的初始化表达式求值
    Value init = evaluate(*o.initializer);
    env->define(o.name.lexeme, init);
}

void Interpreter::visit(const FuncDeclStmt &o)
{
    // 构造Func，传入函数声明和当前环境
    auto func = std::make_shared<Func>(&o, env);
    // 将函数加入当前环境
    env->define(o.name.lexeme, func);
}

void Interpreter::visit(const BlockStmt &o)
{
    auto block_env = std::make_shared<Environment>(env);
    execute_block(o.stmts, block_env);
}

void Interpreter::visit(const IfStmt &o)
{
    Value cond = evaluate(*o.condition);
    if (boolean(cond)) {
        execute(*o.then_stmt);
    } else if (o.else_stmt) {
        execute(*o.else_stmt);
    }
}

void Interpreter::visit(const WhileStmt &o)
{
    while (boolean(evaluate(*o.condition))) {
        try {
            execute(*o.body);
        } catch (const Break &) {
            return; // 捕获 Break 信号，终止循环
        } catch (const Continue &) {}
    }
}

void Interpreter::visit(const ForStmt &o)
{
    auto loop_env = std::make_shared<Environment>(env);
    EnvGuard guard(env, loop_env);

    if (o.initializer) execute(*o.initializer);

    while (boolean(evaluate(*o.condition))) {
        try {
            execute(*o.body);
        } catch (const Break &) {
            return;
        } catch (const Continue &) {}

        if (o.increment) evaluate(*o.increment);
    }
}

void Interpreter::visit(const BreakStmt &)
{
    throw Break{};
}

void Interpreter::visit(const ContinueStmt &)
{
    throw Continue{};
}

void Interpreter::visit(const ReturnStmt &o)
{
    Value val = std::monostate{};
    if (o.value) val = evaluate(*o.value);
    throw Return{val};
}

void Interpreter::visit(const PrintStmt &o)
{
    Value val = evaluate(*o.expr);
    std::cout << stringify(val) << '\n';
}

void Interpreter::resolve(const Expr &expr, int depth)
{
    locals[&expr] = depth;
}

void Interpreter::execute_block(const std::vector<std::unique_ptr<Stmt>> &stmts,
                                std::shared_ptr<Environment> nenv)
{
    EnvGuard guard(env, nenv);
    for (const auto &stmt : stmts) execute(*stmt);
}

void Interpreter::interpret(std::vector<std::unique_ptr<Stmt>> stmts)
{
    unsigned first = kept_stmts.size();
    for (auto &s : stmts) kept_stmts.push_back(std::move(s));

    try {
        for (unsigned i = first; i < kept_stmts.size(); ++i)
            execute(*kept_stmts[i]);
    } catch (const RuntimeError &) {
        // error() 已报告，停止执行剩余语句
    }
}
