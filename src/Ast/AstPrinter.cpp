//
// Created by albert on 22/07/2020.
//
#include "AstPrinter.h"
#include "Ast.h"
#include "Lexer/Token.h"
using namespace lbc;

AstPrinter::AstPrinter(llvm::raw_ostream& os) : m_os(os) {}

std::any AstPrinter::visit(AstProgram* ast) {
    m_os << indent() << "AstProgram" << '\n';
    m_indent++;
    ast->stmtList->accept(this);
    m_indent--;
    return {};
}

std::any AstPrinter::visit(AstStmtList* ast) {
    m_os << indent() << "AstStmtList" << '\n';
    m_indent++;
    for (const auto& stmt: ast->stmts) {
        stmt->accept(this);
    }
    m_indent--;
    return {};
}

std::any AstPrinter::visit(AstAssignStmt* ast) {
    m_os << indent() << "AstAssignStmt" << '\n';
    m_indent++;
    ast->identExpr->accept(this);
    ast->expr->accept(this);
    m_indent--;
    return {};
}

std::any AstPrinter::visit(AstExprStmt* ast) {
    m_os << indent() << "AstExprStmt" << '\n';
    m_indent++;
    ast->expr->accept(this);
    m_indent--;
    return {};
}

std::any AstPrinter::visit(AstVarDecl* ast) {
    m_os << indent() << "AstVarDecl \"" << view_to_stringRef(ast->token->lexeme()) << '\"' << '\n';
    m_indent++;
    if (ast->attribs) {
        ast->attribs->accept(this);
    }
    if (ast->typeExpr) {
        ast->typeExpr->accept(this);
    }
    if (ast->expr) {
        ast->expr->accept(this);
    }
    m_indent--;
    return {};
}

std::any AstPrinter::visit(AstFuncDecl* ast) {
    m_os << indent() << "AstFuncDecl \"" << view_to_stringRef(ast->token->lexeme()) << '\"' << '\n';
    m_indent++;
    if (ast->attribs) {
        ast->attribs->accept(this);
    }

    for (const auto& param: ast->paramDecls) {
        param->accept(this);
    }

    if (ast->retTypeExpr) {
        ast->retTypeExpr->accept(this);
    }
    m_indent--;
    return {};
}

std::any AstPrinter::visit(AstFuncParamDecl* ast) {
    m_os << indent() << "AstFuncParamDecl \"" << view_to_stringRef(ast->token->lexeme()) << '"' << '\n';
    m_indent++;
    if (ast->attribs) {
        ast->attribs->accept(this);
    }
    ast->typeExpr->accept(this);
    m_indent--;
    return {};
}

std::any AstPrinter::visit(AstAttributeList* ast) {
    m_os << indent() << "AstAttributeList" << '\n';
    m_indent++;
    for (const auto& attr: ast->attribs) {
        attr->accept(this);
    }
    m_indent--;
    return {};
}

std::any AstPrinter::visit(AstAttribute* ast) {
    m_os << indent() << "AstAttribute" << '\n';
    m_indent++;
    ast->identExpr->accept(this);
    for (const auto& arg: ast->argExprs) {
        arg->accept(this);
    }
    m_indent--;
    return {};
}

std::any AstPrinter::visit(AstTypeExpr* ast) {
    m_os << indent() << "AstTypeExpr \"" << view_to_stringRef(ast->token->lexeme()) << '"' << '\n';
    return {};
}

std::any AstPrinter::visit(AstIdentExpr* ast) {
    m_os << indent() << "AstIdentExpr \"" << view_to_stringRef(ast->token->lexeme()) << '"' << '\n';
    return {};
}

std::any AstPrinter::visit(AstCallExpr* ast) {
    m_os << indent() << "AstCallExpr" << '\n';
    m_indent++;
    ast->identExpr->accept(this);
    for (const auto& arg: ast->argExprs) {
        arg->accept(this);
    }
    m_indent--;
    return {};
}

std::any AstPrinter::visit(AstLiteralExpr* ast) {
    m_os << indent() << "AstLiteralExpr \"" << view_to_stringRef(ast->token->lexeme()) << '"' << '\n';
    return {};
}

string AstPrinter::indent() const {
    return string(m_indent * SPACES, ' ');
}
