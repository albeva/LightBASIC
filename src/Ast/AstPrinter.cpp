//
// Created by Albert Varaksin on 22/07/2020.
//
#include "AstPrinter.h"
#include "Ast.h"
#include "Lexer/Token.h"
using namespace lbc;

void AstPrinter::visit(AstModule* ast) noexcept {
    m_os << indent() << "AstModule" << '\n';
    m_indent++;
    visit(ast->stmtList.get());
    m_indent--;
}

void AstPrinter::visit(AstStmtList* ast) noexcept {
    m_os << indent() << "AstStmtList" << '\n';
    m_indent++;
    for (const auto& stmt : ast->stmts) {
        visit(stmt.get());
    }
    m_indent--;
}

void AstPrinter::visit(AstAssignStmt* ast) noexcept {
    m_os << indent() << "AstAssignStmt" << '\n';
    m_indent++;
    visit(ast->identExpr.get());
    visit(ast->expr.get());
    m_indent--;
}

void AstPrinter::visit(AstExprStmt* ast) noexcept {
    m_os << indent() << "AstExprStmt" << '\n';
    m_indent++;
    visit(ast->expr.get());
    m_indent--;
}

void AstPrinter::visit(AstVarDecl* ast) noexcept {
    m_os << indent() << "AstVarDecl \"" << ast->id << '\"' << '\n';
    m_indent++;
    if (ast->attributes) {
        visit(ast->attributes.get());
    }
    if (ast->typeExpr) {
        visit(ast->typeExpr.get());
    }
    if (ast->expr) {
        visit(ast->expr.get());
    }
    m_indent--;
}

void AstPrinter::visit(AstFuncDecl* ast) noexcept {
    m_os << indent() << "AstFuncDecl \"" << ast->id << '\"' << '\n';
    m_indent++;
    if (ast->attributes) {
        visit(ast->attributes.get());
    }

    for (const auto& param : ast->paramDecls) {
        visit(param.get());
    }

    if (ast->retTypeExpr) {
        visit(ast->retTypeExpr.get());
    }
    m_indent--;
}

void AstPrinter::visit(AstFuncParamDecl* ast) noexcept {
    m_os << indent() << "AstFuncParamDecl \"" << ast->id << '"' << '\n';
    m_indent++;
    if (ast->attributes) {
        visit(ast->attributes.get());
    }
    visit(ast->typeExpr.get());
    m_indent--;
}

void AstPrinter::visit(AstFuncStmt* /*ast*/) noexcept {
    m_os << indent() << "AstFuncStmt";
}

void AstPrinter::visit(AstReturnStmt* /*ast*/) noexcept {
    m_os << indent() << "AstReturnStmt";
}

void AstPrinter::visit(AstAttributeList* ast) noexcept {
    m_os << indent() << "AstAttributeList" << '\n';
    m_indent++;
    for (const auto& attr : ast->attribs) {
        visit(attr.get());
    }
    m_indent--;
}

void AstPrinter::visit(AstAttribute* ast) noexcept {
    m_os << indent() << "AstAttribute" << '\n';
    m_indent++;
    visit(ast->identExpr.get());
    for (const auto& arg : ast->argExprs) {
        visit(arg.get());
    }
    m_indent--;
}

void AstPrinter::visit(AstTypeExpr* ast) noexcept {
    m_os << indent() << "AstTypeExpr \"" << Token::description(ast->tokenKind) << '"' << '\n';
}

void AstPrinter::visit(AstIdentExpr* ast) noexcept {
    m_os << indent() << "AstIdentExpr \"" << ast->id << '"' << '\n';
}

void AstPrinter::visit(AstCallExpr* ast) noexcept {
    m_os << indent() << "AstCallExpr" << '\n';
    m_indent++;
    visit(ast->identExpr.get());
    for (const auto& arg : ast->argExprs) {
        visit(arg.get());
    }
    m_indent--;
}

void AstPrinter::visit(AstLiteralExpr* /*ast*/) noexcept {
    m_os << indent() << "AstLiteralExpr NOT_IMPLEMENTED \n";
}

void AstPrinter::visit(AstUnaryExpr* /*ast*/) noexcept {
    m_os << indent() << "visitUnaryExpr\n";
}

void AstPrinter::visit(AstCastExpr* /*ast*/) noexcept {
    m_os << indent() << "AstCastExpr" << '\n';
}

string AstPrinter::indent() const noexcept {
    return string(m_indent * SPACES, ' ');
}
