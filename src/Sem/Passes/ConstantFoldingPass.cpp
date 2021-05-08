//
// Created by Albert Varaksin on 05/05/2021.
//
#include "ConstantFoldingPass.h"
#include "Type/Type.h"
using namespace lbc;
using namespace Sem;

void ConstantFoldingPass::fold(unique_ptr<AstExpr>& ast) noexcept {
    unique_ptr<AstExpr> replace;

    switch (ast->kind()) {
    case AstKind::UnaryExpr:
        replace = visitUnaryExpr(static_cast<AstUnaryExpr*>(ast.get())); // NOLINT
        break;
    case AstKind::CastExpr:
        replace = visitCastExpr(static_cast<AstCastExpr*>(ast.get())); // NOLINT
        break;
    default:
        return;
    }

    if (replace != nullptr) {
        ast.swap(replace);
    }
}

unique_ptr<AstExpr> ConstantFoldingPass::visitUnaryExpr(AstUnaryExpr* ast) noexcept {
    auto* literal = dyn_cast<AstLiteralExpr>(ast->expr.get());
    if (literal == nullptr) {
        return nullptr;
    }

    auto replacement = AstLiteralExpr::create();
    replacement->type = literal->type;

    if (ast->tokenKind == TokenKind::Negate) {
        // clang-format off
        std::visit(Overloaded{
            [&](uint64_t integral) {
                replacement->value = static_cast<uint64_t>(-static_cast<int64_t>(integral));
            },
            [&](double fp) {
                replacement->value = -fp;
            },
            [&](auto) {
                llvm_unreachable("Non supported type");
            }
        }, literal->value);
        // clang-format on
    } else {
        llvm_unreachable("Unsupported unary operation");
    }

    return replacement;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitCastExpr(AstCastExpr*  /*ast*/) noexcept {
    return nullptr;
}
