//
// Created by Albert Varaksin on 05/07/2020.
//
#pragma once
#include "Ast.hpp"

namespace lbc {

template<class This, typename ExprTy = void>
class AstVisitor {
public:
    using ExprRetTy = ExprTy;

#define VISIT_CASE(KIND) \
    case AstKind::KIND:  \
        return static_cast<This*>(this)->visit(static_cast<Ast##KIND&>(ast));

    void visit(AstStmt& ast) {
        switch (ast.kind) {
            AST_STMT_NODES(VISIT_CASE)
            AST_DECL_NODES(VISIT_CASE)
        default:
            llvm_unreachable(("visit: Unmatched Stmt: "_t + ast.getClassName()).str().c_str());
        }
    }

    void visit(AstDecl& ast) {
        switch (ast.kind) {
            AST_DECL_NODES(VISIT_CASE)
        default:
            llvm_unreachable(("visit: Unmatched Decl: "_t + ast.getClassName()).str().c_str());
        }
    }

    ExprRetTy visit(AstExpr& ast) {
        switch (ast.kind) {
            AST_EXPR_NODES(VISIT_CASE)
        default:
            llvm_unreachable(("visit: Unmatched Expr: "_t + ast.getClassName()).str().c_str());
        }
    }
#undef VISIT_CASE
};

#define VISIT_METHOD(KIND) void visit(Ast##KIND&);
#define VISIT_METHOD_EXPR(KIND) ExprRetTy visit(Ast##KIND&);

#define AST_VISITOR_DECLARE_CONTENT_FUNCS() \
    using AstVisitor::visit;                \
    AST_BASIC_NODES(VISIT_METHOD)           \
    AST_STMT_NODES(VISIT_METHOD)            \
    AST_DECL_NODES(VISIT_METHOD)            \
    AST_TYPE_NODES(VISIT_METHOD)            \
    AST_EXPR_NODES(VISIT_METHOD_EXPR)

} // namespace lbc
