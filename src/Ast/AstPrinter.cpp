//
// Created by Albert on 05/07/2020.
//
#include "AstPrinter.h"
#include "Ast.h"
#include "Lexer/Token.h"
using namespace lbc;

void AstPrinter::visit(const AstProgram *ast) {
    ast->body->accept(this);
}

void AstPrinter::visit(const AstStmtList *ast) {
    for (const auto& stmt: ast->stmts) {
        stmt->accept(this);
    }
}

void AstPrinter::visit(const AstAssignStmt *ast) {
    ast->ident->accept(this);
    std::cout << " = ";
    ast->expr->accept(this);
    std::cout << '\n';
}

void AstPrinter::visit(const AstExprStmt *ast) {
    ast->expr->accept(this);
    std::cout << '\n';
}

void AstPrinter::visit(const AstVarDecl *ast) {
    std::cout << "VAR ";
    ast->ident->accept(this);
    std::cout << " = ";
    ast->expr->accept(this);
    std::cout << '\n';
}

void AstPrinter::visit(const AstIdentExpr *ast) {
    std::cout << ast->token->lexeme();
}

void AstPrinter::visit(const AstCallExpr *ast) {
    ast->ident->accept(this);
    std::cout << "(";
    bool isFirst = true;
    for (const auto& arg: ast->arguments) {
        if (isFirst) isFirst = false;
        else std::cout << ", ";
        arg->accept(this);
    }
    std::cout << ")";
}

void AstPrinter::visit(const AstLiteralExpr *ast) {
    if (ast->token->kind() == TokenKind::StringLiteral) {
        std::cout << '"' << ast->token->lexeme() << '"';
    } else {
        std::cout << ast->token->lexeme();
    }
}
