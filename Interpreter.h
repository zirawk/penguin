#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "Environment.h"
#include "Expr.h"
#include "Stmt.h"
#include "Token.h"
#include "value.h"
#include <memory>
#include <unordered_map>
#include <vector>

class Interpreter : public ExprVisitor, public StmtVisitor {
    using enum TokenType;
private:
    std::shared_ptr<Environment> env;
    std::shared_ptr<Environment> global;
    std::unordered_map<const Expr *, int> locals;

    std::vector<std::unique_ptr<Stmt>> kept_stmts;

    Value result;

    static double number(const Value &v, const Token &op);
    static Value add(const Value &l, const Value &r, const Token &op);

    Value evaluate(const Expr &expr);
    void execute(const Stmt &stmt);

    // Visit expressions
    virtual void visit(const Assign &o) override;
    virtual void visit(const Logical &o) override;
    virtual void visit(const Binary &o) override;
    virtual void visit(const Unary &o) override;
    virtual void visit(const Call &o) override;
    virtual void visit(const Grouping &o) override;
    virtual void visit(const Literal &o) override;
    virtual void visit(const Variable &o) override;

    // Visit statements
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
    Interpreter();

    // expr 为 Assign/Variable
    // 将 expr 对应的 depth 存入 locals
    void resolve(const Expr &expr, int depth);

    void execute_block(const std::vector<std::unique_ptr<Stmt>> &stmts, std::shared_ptr<Environment> nenv);
    void interpret(std::vector<std::unique_ptr<Stmt>> stmts);
};

struct Return {
    Value value;
};

#endif // INTERPRETER_H
