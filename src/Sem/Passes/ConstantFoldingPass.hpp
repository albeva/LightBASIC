//
// Created by Albert Varaksin on 05/05/2021.
//
#pragma once
#include "pch.hpp"
#include "Ast/Ast.hpp"

namespace lbc {
class Context;
class TypeRoot;

namespace Sem {

    class ConstantFoldingPass final {
    public:
        NO_COPY_AND_MOVE(ConstantFoldingPass)

        explicit ConstantFoldingPass(Context& context) noexcept : m_context{ context } {}
        ~ConstantFoldingPass() noexcept = default;

        void fold(unique_ptr<AstExpr>& ast) noexcept;

    private:
        static unique_ptr<AstExpr> visitUnaryExpr(const AstUnaryExpr* ast) noexcept;
        static AstLiteralExpr::Value unary(TokenKind op, const AstLiteralExpr* ast) noexcept;
        static unique_ptr<AstExpr> visitIfExpr(AstIfExpr* ast) noexcept;
        static unique_ptr<AstExpr> optimizeIifToCast(AstIfExpr* ast) noexcept;
        static unique_ptr<AstExpr> visitBinaryExpr(AstBinaryExpr* ast) noexcept;
        static unique_ptr<AstExpr> visitCastExpr(const AstCastExpr* ast) noexcept;
        static AstLiteralExpr::Value cast(const TypeRoot* type, const AstLiteralExpr* literal) noexcept;

        Context& m_context;
    };

} // namespace Sem
} // namespace lbc
