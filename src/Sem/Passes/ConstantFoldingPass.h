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

        ConstantFoldingPass() noexcept = default;
        ~ConstantFoldingPass() = default;

        void fold(unique_ptr<AstExpr>& ast) noexcept;

    private:
        static unique_ptr<AstExpr> visitUnaryExpr(AstUnaryExpr* ast) noexcept;
        static unique_ptr<AstExpr> visitCastExpr(AstCastExpr* ast) noexcept;
    };

} // namespace Sem
} // namespace lbc
