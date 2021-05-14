//
// Created by Albert Varaksin on 01/05/2021.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"
#include "Driver/Context.h"

namespace lbc {

class SymbolTable;
class Symbol;
class Token;

namespace Sem {

    /**
     * Semantic pass that declares all the functions
     * and declarations in the ast
     */
    class FuncDeclarerPass {
    public:
        NO_COPY_AND_MOVE(FuncDeclarerPass)

        explicit FuncDeclarerPass(Context& context) noexcept : m_context{ context } {}
        ~FuncDeclarerPass() noexcept = default;

        void visit(AstModule* ast) noexcept;

    private:
        void visitFuncDecl(AstFuncDecl* ast, bool external) noexcept;
        void visitFuncParamDecl(AstFuncParamDecl* ast) noexcept;
        static void visitTypeExpr(AstTypeExpr* ast) noexcept;
        [[nodiscard]] Symbol* createParamSymbol(AstFuncParamDecl* ast) noexcept;

        SymbolTable* m_table{};
        Context& m_context;
    };

} // namespace Sem
} // namespace lbc
