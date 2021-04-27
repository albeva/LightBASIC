////
//// Created by Albert Varaksin on 05/07/2020.
////
#include "CodePrinter.h"
#include "Ast.h"
#include "Lexer/Token.h"
using namespace lbc;

void CodePrinter::visitProgram(AstProgram* ast) {
    visitStmtList(ast->stmtList.get());
}

// Statements

void CodePrinter::visitStmtList(AstStmtList* ast) {
    for (const auto& stmt : ast->stmts) {
        visitStmt(stmt.get());
        m_os << '\n';
    }
}

void CodePrinter::visitAssignStmt(AstAssignStmt* ast) {
    visitIdentExpr(ast->identExpr.get());
    m_os << " = ";
    visitExpr(ast->expr.get());
}

void CodePrinter::visitExprStmt(AstExprStmt* ast) {
    visitExpr(ast->expr.get());
}

// Attributes

void CodePrinter::visitAttributeList(AstAttributeList* ast) {
    m_os << '[';
    bool isFirst = true;
    for (const auto& attr : ast->attribs) {
        if (isFirst) {
            isFirst = false;
        } else {
            m_os << ", ";
        }
        visitAttribute(attr.get());
    }
    m_os << "]";
}

void CodePrinter::visitAttribute(AstAttribute* ast) {
    visitIdentExpr(ast->identExpr.get());
    if (ast->argExprs.size() == 1) {
        m_os << " = ";
        visitLiteralExpr(ast->argExprs[0].get());
    } else if (ast->argExprs.size() > 1) {
        bool isFirst = true;
        m_os << "(";
        for (const auto& arg : ast->argExprs) {
            if (isFirst) {
                isFirst = false;
            } else {
                m_os << ", ";
            }
            visitLiteralExpr(arg.get());
        }
        m_os << ")";
    }
}

void CodePrinter::visitTypeExpr(AstTypeExpr* ast) {
    m_os << ast->token->lexeme();
}

// Declarations

void CodePrinter::visitVarDecl(AstVarDecl* ast) {
    if (ast->attributes) {
        visitAttributeList(ast->attributes.get());
        m_os << " _" << '\n';
    }

    m_os << "VAR ";
    m_os << ast->token->lexeme();

    if (ast->typeExpr) {
        m_os << " AS ";
        visitTypeExpr(ast->typeExpr.get());
    }

    if (ast->expr) {
        m_os << " = ";
        visitExpr(ast->expr.get());
    }
}

void CodePrinter::visitFuncDecl(AstFuncDecl* ast) {
    if (ast->attributes) {
        visitAttributeList(ast->attributes.get());
        m_os << " _" << '\n';
    }

    if (ast->retTypeExpr) {
        m_os << "FUNCTION ";
    } else {
        m_os << "SUB ";
    }
    m_os << ast->token->lexeme();

    if (!ast->paramDecls.empty()) {
        m_os << "(";
        bool isFirst = true;
        for (const auto& param : ast->paramDecls) {
            if (isFirst) {
                isFirst = false;
            } else {
                m_os << ", ";
            }
            visitFuncParamDecl(param.get());
        }
        m_os << ")";
    }

    if (ast->retTypeExpr) {
        m_os << " AS ";
        visitTypeExpr(ast->retTypeExpr.get());
    }
}

void CodePrinter::visitFuncParamDecl(AstFuncParamDecl* ast) {
    m_os << ast->token->lexeme();
    m_os << " AS ";
    visitTypeExpr(ast->typeExpr.get());
}

void CodePrinter::visitFuncStmt(AstFuncStmt* ast) {
    m_os << indent() << "AstFuncStmt";
}

// Expressions

void CodePrinter::visitIdentExpr(AstIdentExpr* ast) {
    m_os << ast->token->lexeme();
}

void CodePrinter::visitCallExpr(AstCallExpr* ast) {
    visitIdentExpr(ast->identExpr.get());
    m_os << "(";
    bool isFirst = true;
    for (const auto& arg : ast->argExprs) {
        if (isFirst) {
            isFirst = false;
        } else {
            m_os << ", ";
        }

        visitExpr(arg.get());
    }
    m_os << ")";
}

void CodePrinter::visitLiteralExpr(AstLiteralExpr* ast) {
    if (ast->token->kind() == TokenKind::StringLiteral) {
        m_os << '"' << ast->token->lexeme() << '"';
    } else {
        m_os << ast->token->lexeme();
    }
}

string CodePrinter::indent() const {
    return string(m_indent * SPACES, ' ');
}
