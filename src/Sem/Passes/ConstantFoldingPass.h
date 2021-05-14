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

    class ConstantFoldingPass final {
    public:
        NO_COPY_AND_MOVE(ConstantFoldingPass)

        explicit ConstantFoldingPass(Context& context) noexcept : m_context{ context } {}
        ~ConstantFoldingPass() = default;

        void fold(unique_ptr<AstExpr>& ast) noexcept;

    private:
        static unique_ptr<AstExpr> visitUnaryExpr(AstUnaryExpr* ast) noexcept;
        static unique_ptr<AstExpr> visitIfExpr(AstIfExpr* ast) noexcept;
        static unique_ptr<AstExpr> optimizeIifToCast(AstIfExpr* ast) noexcept;
        static unique_ptr<AstExpr> visitBinaryExpr(AstBinaryExpr* ast) noexcept;
        static unique_ptr<AstExpr> visitCastExpr(AstCastExpr* ast) noexcept;

        Context& m_context;
    };

} // namespace Sem
} // namespace lbc
