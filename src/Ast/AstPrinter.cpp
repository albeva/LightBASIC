//
// Created by Albert Varaksin on 22/07/2020.
//
#include "AstPrinter.h"
#include "Ast.h"
#include "Lexer/Token.h"
using namespace lbc;

void AstPrinter::visitProgram(AstProgram* ast) {
    m_os << indent() << "AstProgram" << '\n';
    m_indent++;
    visitStmtList(ast->stmtList.get());
    m_indent--;
}

void AstPrinter::visitStmtList(AstStmtList* ast) {
    m_os << indent() << "AstStmtList" << '\n';
    m_indent++;
    for (const auto& stmt : ast->stmts) {
        visitStmt(stmt.get());
    }
    m_indent--;
}

void AstPrinter::visitAssignStmt(AstAssignStmt* ast) {
    m_os << indent() << "AstAssignStmt" << '\n';
    m_indent++;
    visitIdentExpr(ast->identExpr.get());
    visitExpr(ast->expr.get());
    m_indent--;
}

void AstPrinter::visitExprStmt(AstExprStmt* ast) {
    m_os << indent() << "AstExprStmt" << '\n';
    m_indent++;
    visitExpr(ast->expr.get());
    m_indent--;
}

void AstPrinter::visitVarDecl(AstVarDecl* ast) {
    m_os << indent() << "AstVarDecl \"" << view_to_stringRef(ast->token->lexeme()) << '\"' << '\n';
    m_indent++;
    if (ast->attributes) {
        visitAttributeList(ast->attributes.get());
    }
    if (ast->typeExpr) {
        visitTypeExpr(ast->typeExpr.get());
    }
    if (ast->expr) {
        visitExpr(ast->expr.get());
    }
    m_indent--;
}

void AstPrinter::visitFuncDecl(AstFuncDecl* ast) {
    m_os << indent() << "AstFuncDecl \"" << view_to_stringRef(ast->token->lexeme()) << '\"' << '\n';
    m_indent++;
    if (ast->attributes) {
        visitAttributeList(ast->attributes.get());
    }

    for (const auto& param : ast->paramDecls) {
        visitFuncParamDecl(param.get());
    }

    if (ast->retTypeExpr) {
        visitTypeExpr(ast->retTypeExpr.get());
    }
    m_indent--;
}

void AstPrinter::visitFuncParamDecl(AstFuncParamDecl* ast) {
    m_os << indent() << "AstFuncParamDecl \"" << view_to_stringRef(ast->token->lexeme()) << '"' << '\n';
    m_indent++;
    if (ast->attributes) {
        visitAttributeList(ast->attributes.get());
    }
    visitTypeExpr(ast->typeExpr.get());
    m_indent--;
}

void AstPrinter::visitAttributeList(AstAttributeList* ast) {
    m_os << indent() << "AstAttributeList" << '\n';
    m_indent++;
    for (const auto& attr : ast->attribs) {
        visitAttribute(attr.get());
    }
    m_indent--;
}

void AstPrinter::visitAttribute(AstAttribute* ast) {
    m_os << indent() << "AstAttribute" << '\n';
    m_indent++;
    visitIdentExpr(ast->identExpr.get());
    for (const auto& arg : ast->argExprs) {
        visitLiteralExpr(arg.get());
    }
    m_indent--;
}

void AstPrinter::visitTypeExpr(AstTypeExpr* ast) {
    m_os << indent() << "AstTypeExpr \"" << view_to_stringRef(ast->token->lexeme()) << '"' << '\n';
}

void AstPrinter::visitIdentExpr(AstIdentExpr* ast) {
    m_os << indent() << "AstIdentExpr \"" << view_to_stringRef(ast->token->lexeme()) << '"' << '\n';
}

void AstPrinter::visitCallExpr(AstCallExpr* ast) {
    m_os << indent() << "AstCallExpr" << '\n';
    m_indent++;
    visitIdentExpr(ast->identExpr.get());
    for (const auto& arg : ast->argExprs) {
        visitExpr(arg.get());
    }
    m_indent--;
}

void AstPrinter::visitLiteralExpr(AstLiteralExpr* ast) {
    m_os << indent() << "AstLiteralExpr \"" << view_to_stringRef(ast->token->lexeme()) << '"' << '\n';
}

string AstPrinter::indent() const {
    return string(m_indent * SPACES, ' ');
}
