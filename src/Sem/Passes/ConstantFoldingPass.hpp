//
// Created by Albert Varaksin on 05/05/2021.
//
#pragma once
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

        void fold(AstExpr*& ast);

    private:
        static AstExpr* visitUnaryExpr(const AstUnaryExpr& ast);
        static AstLiteralExpr::Value unary(TokenKind op, const AstLiteralExpr& ast);
        static AstExpr* visitIfExpr(AstIfExpr& ast);
        static AstExpr* optimizeIifToCast(AstIfExpr& ast);
        static AstExpr* visitBinaryExpr(AstBinaryExpr& ast);
        static AstExpr* visitCastExpr(const AstCastExpr& ast);
        static AstLiteralExpr::Value cast(const TypeRoot* type, const AstLiteralExpr& ast);

        Context& m_context;
    };

} // namespace Sem
} // namespace lbc
