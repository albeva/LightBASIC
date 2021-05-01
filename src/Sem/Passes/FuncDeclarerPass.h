//
// Created by Albert on 01/05/2021.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"
#include "Driver/Context.h"

namespace lbc {

class SymbolTable;
class Symbol;
class Token;

class FuncDeclarerPass: private NonCopyable {
public:
    explicit FuncDeclarerPass(Context& context) noexcept;
    void visit(AstModule* ast) noexcept;

private:
    void visitFuncDecl(AstFuncDecl* ast) noexcept;
    void visitFuncParamDecl(AstFuncParamDecl* ast) noexcept;
    void visitTypeExpr(AstTypeExpr* ast) noexcept;
    [[nodiscard]] Symbol* createParamSymbol(AstFuncParamDecl* ast) noexcept;

    SymbolTable* m_table;
    Context& m_context;
};

} // namespace lbc
