#include "Func.h"
#include "Environment.h"
#include "Interpreter.h"
#include <cassert>
#include <variant>

Func::Func(const FuncDeclStmt *decl_, Environment *closure_)
    : decl(decl_), closure(closure_) {}

unsigned Func::arity()
{
    return decl->params.size();
}

Value Func::call(Interpreter &interp, const std::vector<Value> &args)
{
    assert(args.size() <= decl->params.size());

    // 环境准备
    Environment call_env(closure);
    for (unsigned i = 0; i < args.size(); ++i)
        call_env.define(decl->params[i].lexeme, args[i]);
    for (unsigned i = args.size(); i < arity(); ++i)
        call_env.define(decl->params[i].lexeme, std::monostate{});

    try {
        interp.execute_block(decl->body, call_env);
    } catch (const Return &ret) {
        return ret.value;
    }

    return std::monostate{};
}

std::string Func::str()
{
    return "<fn " + decl->name.lexeme + ">";
}

