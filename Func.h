#ifndef FUNC_H
#define FUNC_H

#include "Environment.h"
#include "value.h"
#include "Stmt.h"
#include <memory>

class Interpreter;

class Func : public Callable {
private:
    const FuncDeclStmt *decl;
    std::shared_ptr<Environment> closure;
public:
    Func(const FuncDeclStmt *decl_, std::shared_ptr<Environment> closure_);
    virtual unsigned arity() override;
    virtual Value call(Interpreter &interp, const std::vector<Value> &args) override;
    virtual std::string str() override;
};

#endif // FUNC_H
