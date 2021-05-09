//
// Created by Albert Varaksin on 22/07/2020.
//
#include "AstPrinter.h"
#include "Ast.h"
#include "Driver/Context.h"
#include "Lexer/Token.h"
using namespace lbc;

void AstPrinter::visit(AstModule* ast) noexcept {
    m_os << indent() << "AstModule" << '\n';
    m_indent++;
    visit(ast->stmtList.get());
    m_indent--;
}

void AstPrinter::visit(AstStmtList* ast) noexcept {
    m_os << indent() << "AstStmtList " << range(ast) << '\n';
    m_indent++;
    for (const auto& stmt : ast->stmts) {
        visit(stmt.get());
    }
    m_indent--;
}

void AstPrinter::visit(AstAssignStmt* ast) noexcept {
    m_os << indent() << "AstAssignStmt " << range(ast) << '\n';
    m_indent++;
    visit(ast->identExpr.get());
    visit(ast->expr.get());
    m_indent--;
}

void AstPrinter::visit(AstExprStmt* ast) noexcept {
    m_os << indent() << "AstExprStmt " << range(ast) << '\n';
    m_indent++;
    visit(ast->expr.get());
    m_indent--;
}

void AstPrinter::visit(AstVarDecl* ast) noexcept {
    m_os << indent() << "AstVarDecl \"" << ast->id << "\" " << range(ast) << '\n';
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
    m_os << indent() << "AstFuncDecl \"" << ast->id << "\" " << range(ast) << '\n';
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
    m_os << indent() << "AstFuncParamDecl \"" << ast->id << "\" " << range(ast) << '\n';
    m_indent++;
    if (ast->attributes) {
        visit(ast->attributes.get());
    }
    visit(ast->typeExpr.get());
    m_indent--;
}

void AstPrinter::visit(AstFuncStmt* ast) noexcept {
    m_os << indent() << "AstFuncStmt " << range(ast) << '\n';
}

void AstPrinter::visit(AstReturnStmt* ast) noexcept {
    m_os << indent() << "AstReturnStmt " << range(ast) << '\n';
}

void AstPrinter::visit(AstAttributeList* ast) noexcept {
    m_os << indent() << "AstAttributeList " << range(ast) << '\n';
    m_indent++;
    for (const auto& attr : ast->attribs) {
        visit(attr.get());
    }
    m_indent--;
}

void AstPrinter::visit(AstAttribute* ast) noexcept {
    m_os << indent() << "AstAttribute " << range(ast) << '\n';
    m_indent++;
    visit(ast->identExpr.get());
    for (const auto& arg : ast->argExprs) {
        visit(arg.get());
    }
    m_indent--;
}

void AstPrinter::visit(AstTypeExpr* ast) noexcept {
    m_os << indent() << "AstTypeExpr \"" << Token::description(ast->tokenKind) << "\" " << range(ast) << '\n';
}

void AstPrinter::visit(AstIdentExpr* ast) noexcept {
    m_os << indent() << "AstIdentExpr \"" << ast->id << "\" " << range(ast) << '\n';
}

void AstPrinter::visit(AstCallExpr* ast) noexcept {
    m_os << indent() << "AstCallExpr " << range(ast) << '\n';
    m_indent++;
    visit(ast->identExpr.get());
    for (const auto& arg : ast->argExprs) {
        visit(arg.get());
    }
    m_indent--;
}

void AstPrinter::visit(AstLiteralExpr* ast) noexcept {
    m_os << indent() << "AstLiteralExpr " << range(ast) << '\n';
}

void AstPrinter::visit(AstUnaryExpr* ast) noexcept {
    m_os << indent() << "visitUnaryExpr " << range(ast) << '\n';
}

void AstPrinter::visit(AstCastExpr* ast) noexcept {
    m_os << indent() << "AstCastExpr " << range(ast) << '\n';
}

string AstPrinter::indent() const noexcept {
    return string(m_indent * SPACES, ' ');
}

string AstPrinter::range(AstRoot* ast) const noexcept {
    auto [fromLine, fromCol] = m_context.getSourceMrg().getLineAndColumn(ast->getRange().Start);
    auto [untilLine, untilCol] = m_context.getSourceMrg().getLineAndColumn(ast->getRange().End);

    if (fromLine == untilLine) {
        return llvm::formatv("<line:{0}:{1}, col:{2}>", fromLine, fromCol, untilCol);
    }
    return llvm::formatv("<line:{0}:{1}, line:{2}:{3}>", fromLine, fromCol, untilLine, untilCol);
}
