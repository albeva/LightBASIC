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
    if (!ast->expr->constant) {
        return nullptr;
    }

    auto operation = ast->token->kind();

    auto replacement = AstLiteralExpr::create();
    replacement->type = ast->expr->type;
    replacement->constant = true;

    switch (operation) {
    case TokenKind::Negate:
        if (isa<TypeIntegral>(ast->expr->type)) {
            replacement->value.int64 = -(ast->expr->value.int64);
            replacement->token = Token::create(TokenKind::IntegerLiteral, ast->token->loc());
        } else if (isa<TypeFloatingPoint>(ast->expr->type)) {
            replacement->value.dbl = -(ast->expr->value.dbl);
            replacement->token = Token::create(TokenKind::FloatingPointLiteral, ast->token->loc());
        }
        break;
    default:
        break;
    }

    return replacement;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitCastExpr(AstCastExpr* ast) {
    return nullptr;
}

