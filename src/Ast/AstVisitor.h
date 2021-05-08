//
// Created by Albert Varaksin on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast.h"

namespace lbc {

template<class This>
class AstVisitor {
public:
#define VISIT_CASE(KIND) \
    case AstKind::KIND: \
        return static_cast<This*>(this)->visit(static_cast<Ast##KIND*>(ast));

    void visit(AstStmt* ast) noexcept {
        switch (ast->kind()) {
            AST_STMT_NODES(VISIT_CASE)
            AST_DECL_NODES(VISIT_CASE)
        default:
            llvm_unreachable(("visit: Unmatched Stmt: "_t + ast->describe()).str().c_str());
        }
    }

    void visit(AstDecl* ast) noexcept {
        switch (ast->kind()) {
        AST_DECL_NODES(VISIT_CASE)
        default:
            llvm_unreachable(("visit: Unmatched Decl: "_t + ast->describe()).str().c_str());
        }
    }

    void visit(AstAttr* ast) noexcept {
        switch (ast->kind()) {
        AST_ATTRIB_NODES(VISIT_CASE)
        default:
            llvm_unreachable(("visit: Unmatched Attr: "_t + ast->describe()).str().c_str());
        }
    }

    void visit(AstType* ast) noexcept {
        switch (ast->kind()) {
        AST_TYPE_NODES(VISIT_CASE)
        default:
            llvm_unreachable(("visit: Unmatched Attr: "_t + ast->describe()).str().c_str());
        }
    }

    void visit(AstExpr* ast) noexcept {
        switch (ast->kind()) {
        AST_EXPR_NODES(VISIT_CASE)
        default:
            llvm_unreachable(("visit: Unmatched Expr: "_t + ast->describe()).str().c_str());
        }
    }
#undef VISIT_CASE
};

#define VISIT_METHOD_FINAL(KIND) void visit(Ast##KIND*) noexcept;

#define AST_VISITOR_DECLARE_CONTENT_FUNCS() \
    using AstVisitor::visit;                \
    AST_CONTENT_NODES(VISIT_METHOD_FINAL)

} // namespace lbc
