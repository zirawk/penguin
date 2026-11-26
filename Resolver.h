#ifndef RESOLVER_H
#define RESOLVER_H

#include "Expr.h"
#include "Interpreter.h"
#include "Stmt.h"
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

class Resolver : public ExprVisitor, public StmtVisitor {
private:
    Interpreter &interpreter;
    std::vector<std::unordered_set<std::string>> scopes;

    enum class FuncType {
        NONE, FUNCTION
    } current_func;
    using enum Resolver::FuncType;

    bool in_loop;

    void begin_scope();
    void end_scope();
    void define (const Token &name);

    // 从 Assign/Variable 的 visit() 调用
    // 因此有 name.type == IDENTIFIER
    // 若 name 对应局部变量，调用 Interpreter::resolve 将 expr 与嵌套深度绑定
    // 否则不作绑定，运行时在 global 查找
    void resolve_local(const Expr &expr, const Token &name);

    void resolve_function(const FuncDeclStmt &func, FuncType type);

    virtual void visit(const Assign &o) override;
    virtual void visit(const Logical &o) override;
    virtual void visit(const Binary &o) override;
    virtual void visit(const Unary &o) override;
    virtual void visit(const Call &o) override;
    virtual void visit(const Grouping &o) override;
    virtual void visit(const Literal &o) override;
    virtual void visit(const Variable &o) override;

    virtual void visit(const ExprStmt &o) override;
    virtual void visit(const VarDeclStmt &o) override;
    virtual void visit(const FuncDeclStmt &o) override;
    virtual void visit(const BlockStmt &o) override;
    virtual void visit(const IfStmt &o) override;
    virtual void visit(const WhileStmt &o) override;
    virtual void visit(const ForStmt &o) override;
    virtual void visit(const BreakStmt &o) override;
    virtual void visit(const ContinueStmt &o) override;
    virtual void visit(const ReturnStmt &o) override;
    virtual void visit(const PrintStmt &o) override;
public:
    Resolver(Interpreter &interp);

    void resolve(const std::unique_ptr<Expr> &expr);
    void resolve(const std::unique_ptr<Stmt> &stmt);
    void resolve(const std::vector<std::unique_ptr<Stmt>> &stmts);

};

#endif // RESOLVER_H
