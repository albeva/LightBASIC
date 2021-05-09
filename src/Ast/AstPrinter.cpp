//
// Created by Albert Varaksin on 22/07/2020.
//
#include "AstPrinter.h"
#include "Ast.h"
#include "Driver/Context.h"
#include "Lexer/Token.h"
using namespace lbc;

AstPrinter::AstPrinter(Context& context, llvm::raw_ostream& os) noexcept
: m_context{ context }, m_json{ os, 4 } {
}


void AstPrinter::visit(AstModule* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeStmts(ast->stmtList.get());
    });
}

void AstPrinter::visit(AstStmtList* ast) noexcept {
    m_json.array([&] {
        for (const auto& stmt : ast->stmts) {
            visit(stmt.get());
        }
    });
}

void AstPrinter::visit(AstAssignStmt* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeIdent(ast->identExpr.get());
        writeExpr(ast->expr.get());
    });
}

void AstPrinter::visit(AstExprStmt* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeExpr(ast->expr.get());
    });
}

void AstPrinter::visit(AstVarDecl* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeAttributes(ast->attributes.get());
        m_json.attribute("id", ast->id);
        writeType(ast->typeExpr.get());
        writeExpr(ast->expr.get());
    });
}

void AstPrinter::visit(AstFuncDecl* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        m_json.attribute("id", ast->id);
        writeAttributes(ast->attributes.get());

        if (!ast->paramDecls.empty()) {
            m_json.attributeArray("params", [&] {
                for (const auto& param : ast->paramDecls) {
                    visit(param.get());
                }
            });
        }

        writeType(ast->retTypeExpr.get());
    });
}

void AstPrinter::visit(AstFuncParamDecl* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeAttributes(ast->attributes.get());
        m_json.attribute("id", ast->id);
        writeType(ast->typeExpr.get());
    });
}

void AstPrinter::visit(AstFuncStmt* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);

        m_json.attributeBegin("decl");
        visit(ast->decl.get());
        m_json.attributeEnd();

        writeStmts(ast->stmtList.get());
    });
}

void AstPrinter::visit(AstReturnStmt* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeExpr(ast->expr.get());
    });
}

void AstPrinter::visit(AstAttributeList* ast) noexcept {
    m_json.array([&] {
        for (const auto& attr : ast->attribs) {
            visit(attr.get());
        }
    });
}

void AstPrinter::visit(AstAttribute* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeIdent(ast->identExpr.get());
        if (!ast->argExprs.empty()) {
            m_json.attributeArray("args", [&]{
                for (const auto& arg : ast->argExprs) {
                    visit(arg.get());
                }
            });
        }
    });
}

void AstPrinter::visit(AstTypeExpr* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        m_json.attribute("id", Token::description(ast->tokenKind));
    });
}

void AstPrinter::visit(AstIdentExpr* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        m_json.attribute("id", ast->id);
    });
}

void AstPrinter::visit(AstCallExpr* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeIdent(ast->identExpr.get());
        if (!ast->argExprs.empty()) {
            m_json.attributeArray("args", [&]{
                for (const auto& arg: ast->argExprs) {
                    visit(arg.get());
                }
            });
        }
    });
}

void AstPrinter::visit(AstLiteralExpr* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        using Ret = std::pair<TokenKind, string>;
        constexpr auto visitor = Visitor{
            [](std::monostate) -> Ret {
                return {TokenKind::NullLiteral, "null"};
            },
            [](const StringRef& value) -> Ret {
                return {TokenKind::StringLiteral, value.str()};
            },
            [](uint64_t value) -> Ret {
                return {TokenKind::IntegerLiteral, std::to_string(value)};
            },
            [](double value) -> Ret {
                return {TokenKind::FloatingPointLiteral, std::to_string(value)};
            },
            [](bool value) -> Ret {
                return {TokenKind::BooleanLiteral, std::to_string(value)};
            }
        };
        auto [kind, value] = std::visit(visitor, ast->value);
        m_json.attribute("kind", Token::description(kind));
        m_json.attribute("value", value);
    });
}

void AstPrinter::visit(AstUnaryExpr* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeExpr(ast->expr.get());
    });
}

void AstPrinter::visit(AstCastExpr* ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeExpr(ast->expr.get());
    });
}

void AstPrinter::writeAttributes(AstAttributeList* ast) noexcept {
    if (ast == nullptr || ast->attribs.empty()) {
        return;
    }
    m_json.attributeBegin("attrs");
    visit(ast);
    m_json.attributeEnd();
}

void AstPrinter::writeStmts(AstStmtList* ast) noexcept {
    if (ast == nullptr || ast->stmts.empty()) {
        return;
    }
    m_json.attributeBegin("stmts");
    visit(ast);
    m_json.attributeEnd();
}

void AstPrinter::writeHeader(AstRoot* ast) noexcept {
    if (ast == nullptr) {
        return;
    }
    m_json.attribute("class", ast->getClassName());
    m_json.attributeBegin("loc");
    writeLocation(ast);
    m_json.attributeEnd();
}

void AstPrinter::writeExpr(AstExpr* ast) noexcept {
    if (ast == nullptr) {
        return;
    }
    m_json.attributeBegin("expr");
    visit(ast);
    m_json.attributeEnd();
}

void AstPrinter::writeLocation(AstRoot* ast) noexcept {
    auto [startLine, startCol] = m_context.getSourceMrg().getLineAndColumn(ast->getRange().Start);
    auto [endLine, endCol] = m_context.getSourceMrg().getLineAndColumn(ast->getRange().End);

    if (startLine == endLine) {
        m_json.value(llvm::formatv("{0}:{1} - {2}", startLine, startCol, endCol));
    } else {
        m_json.value(llvm::formatv("{0}:{1} - {2}:3", startLine, startCol, endLine, endCol));
    }
}

void AstPrinter::writeIdent(AstIdentExpr* ast) noexcept {
    if (ast == nullptr) {
        return;
    }
    m_json.attributeBegin("ident");
    visit(ast);
    m_json.attributeEnd();
}

void AstPrinter::writeType(AstTypeExpr* ast) noexcept {
    if (ast == nullptr) {
        return;
    }
    m_json.attributeBegin("type");
    visit(ast);
    m_json.attributeEnd();
}
