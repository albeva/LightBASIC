//
// Created by Albert Varaksin on 05/05/2021.
//
#pragma once
#include "pch.h"
#include "Ast/Ast.h"
#include "Ast/AstVisitor.h"

namespace lbc {

class Context;

namespace Sem {

    class ConstantFoldingPass : public AstExprVisitor<ConstantFoldingPass, unique_ptr<AstExpr>> {
    public:
        explicit ConstantFoldingPass(Context& context) noexcept : m_context{ context } {}
        void fold(unique_ptr<AstExpr>& ast) noexcept;

        AST_DECLARE_ALL_EXPR_VISIT_METHODS()
    private:
        Context& m_context;
    };

} // namespace Sem
} // namespace lbc
