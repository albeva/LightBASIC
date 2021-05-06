//
// Created by Albert Varaksin on 05/05/2021.
//
#include "ConstantFoldingPass.h"
#include "Type/Type.h"
using namespace lbc;
using namespace Sem;

void ConstantFoldingPass::fold(unique_ptr<AstExpr>& ast) noexcept {
    if (auto replace = visitExpr(ast.get())) {
        ast.swap(replace);
    }
}

unique_ptr<AstExpr> ConstantFoldingPass::visitIdentExpr(AstIdentExpr* ast) {
    return nullptr;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitCallExpr(AstCallExpr* ast) {
    return nullptr;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitLiteralExpr(AstLiteralExpr* ast) {
    return nullptr;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitUnaryExpr(AstUnaryExpr* ast) {
    auto* literal = dyn_cast<AstLiteralExpr>(ast->expr.get());
    if (literal == nullptr) {
        return nullptr;
    }

    auto operation = ast->tokenKind;

    auto replacement = AstLiteralExpr::create();
    replacement->type = literal->type;

    if (operation == TokenKind::Negate) {
        if (auto* integral = std::get_if<uint64_t>(&literal->value)) {
            replacement->value = -*integral;
        } else if (auto* fp = std::get_if<double>(&literal->value)) {
            replacement->value = -*fp;
        }
    } else {
        llvm_unreachable("Unsupported unary operation");
    }

    return replacement;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitCastExpr(AstCastExpr* ast) {
    return nullptr;
}

