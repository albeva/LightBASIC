////
//// Created by Albert Varaksin on 05/07/2020.
////
#include "CodePrinter.h"
#include "Ast.h"
#include "Lexer/Token.h"
using namespace lbc;

void CodePrinter::visit(AstModule* ast) noexcept {
    visit(ast->stmtList.get());
}

// Statements

void CodePrinter::visit(AstStmtList* ast) noexcept {
    for (const auto& stmt : ast->stmts) {
        visit(stmt.get());
        m_os << '\n';
    }
}

void CodePrinter::visit(AstAssignStmt* ast) noexcept {
    visit(ast->identExpr.get());
    m_os << " = ";
    visit(ast->expr.get());
}

void CodePrinter::visit(AstExprStmt* ast) noexcept {
    visit(ast->expr.get());
}

// Attributes

void CodePrinter::visit(AstAttributeList* ast) noexcept {
    m_os << '[';
    bool isFirst = true;
    for (const auto& attr : ast->attribs) {
        if (isFirst) {
            isFirst = false;
        } else {
            m_os << ", ";
        }
        visit(attr.get());
    }
    m_os << "]";
}

void CodePrinter::visit(AstAttribute* ast) noexcept {
    visit(ast->identExpr.get());
    if (ast->argExprs.size() == 1) {
        m_os << " = ";
        visit(ast->argExprs[0].get());
    } else if (ast->argExprs.size() > 1) {
        bool isFirst = true;
        m_os << "(";
        for (const auto& arg : ast->argExprs) {
            if (isFirst) {
                isFirst = false;
            } else {
                m_os << ", ";
            }
            visit(arg.get());
        }
        m_os << ")";
    }
}

void CodePrinter::visit(AstTypeExpr* ast) noexcept {
    m_os << Token::description(ast->tokenKind);
}

// Declarations

void CodePrinter::visit(AstVarDecl* ast) noexcept {
    if (ast->attributes) {
        visit(ast->attributes.get());
        m_os << " _" << '\n';
    }

    m_os << "VAR ";
    m_os << ast->id;

    if (ast->typeExpr) {
        m_os << " AS ";
        visit(ast->typeExpr.get());
    }

    if (ast->expr) {
        m_os << " = ";
        visit(ast->expr.get());
    }
}

void CodePrinter::visit(AstFuncDecl* ast) noexcept {
    if (ast->attributes) {
        visit(ast->attributes.get());
        m_os << " _" << '\n';
    }

    if (ast->retTypeExpr) {
        m_os << "FUNCTION ";
    } else {
        m_os << "SUB ";
    }
    m_os << ast->id;

    if (!ast->paramDecls.empty()) {
        m_os << "(";
        bool isFirst = true;
        for (const auto& param : ast->paramDecls) {
            if (isFirst) {
                isFirst = false;
            } else {
                m_os << ", ";
            }
            visit(param.get());
        }
        m_os << ")";
    }

    if (ast->retTypeExpr) {
        m_os << " AS ";
        visit(ast->retTypeExpr.get());
    }
}

void CodePrinter::visit(AstFuncParamDecl* ast) noexcept {
    m_os << ast->id;
    m_os << " AS ";
    visit(ast->typeExpr.get());
}

void CodePrinter::visit(AstFuncStmt* /*ast*/) noexcept {
    m_os << indent() << "AstFuncStmt";
}

void CodePrinter::visit(AstReturnStmt* /*ast*/) noexcept {
    m_os << indent() << "RETURN";
}

// Expressions

void CodePrinter::visit(AstIdentExpr* ast) noexcept {
    m_os << ast->id;
}

void CodePrinter::visit(AstCallExpr* ast) noexcept {
    visit(ast->identExpr.get());
    m_os << "(";
    bool isFirst = true;
    for (const auto& arg : ast->argExprs) {
        if (isFirst) {
            isFirst = false;
        } else {
            m_os << ", ";
        }

        visit(arg.get());
    }
    m_os << ")";
}

void CodePrinter::visit(AstLiteralExpr* /*ast*/) noexcept {
    // TODO
}

void CodePrinter::visit(AstUnaryExpr* /*ast*/) noexcept {
    // TODO
}

void CodePrinter::visit(AstCastExpr* /*ast*/) noexcept {
    // TODO
}

string CodePrinter::indent() const noexcept {
    return string(m_indent * SPACES, ' ');
}
