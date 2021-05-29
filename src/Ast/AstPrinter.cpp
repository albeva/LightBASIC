//
// Created by Albert Varaksin on 22/07/2020.
//
#include "AstPrinter.hpp"
#include "Ast.hpp"
#include "Driver/Context.hpp"
#include "Lexer/Token.hpp"
#include "Type/Type.hpp"
using namespace lbc;

AstPrinter::AstPrinter(Context& context, llvm::raw_ostream& os) noexcept
: m_context{ context }, m_json{ os, 4 } {
}

void AstPrinter::visit(AstModule& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeStmts(ast.stmtList.get());
    });
}

void AstPrinter::visit(AstStmtList& ast) noexcept {
    m_json.array([&] {
        for (const auto& stmt : ast.stmts) {
            visit(*stmt);
        }
    });
}

void AstPrinter::visit(AstAssignStmt& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeExpr(ast.lhs.get(), "lhs");
        writeExpr(ast.rhs.get(), "rhs");
    });
}

void AstPrinter::visit(AstExprStmt& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeExpr(ast.expr.get());
    });
}

void AstPrinter::visit(AstVarDecl& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeAttributes(ast.attributes.get());
        m_json.attribute("id", ast.name);
        writeType(ast.typeExpr.get());
        writeExpr(ast.expr.get());
    });
}

void AstPrinter::visit(AstFuncDecl& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        m_json.attribute("id", ast.name);
        writeAttributes(ast.attributes.get());

        if (!ast.paramDecls.empty()) {
            m_json.attributeArray("params", [&] {
                for (const auto& param : ast.paramDecls) {
                    visit(*param);
                }
            });
        }

        writeType(ast.retTypeExpr.get());
    });
}

void AstPrinter::visit(AstFuncParamDecl& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeAttributes(ast.attributes.get());
        m_json.attribute("id", ast.name);
        writeType(ast.typeExpr.get());
    });
}

void AstPrinter::visit(AstFuncStmt& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);

        m_json.attributeBegin("decl");
        visit(*ast.decl);
        m_json.attributeEnd();

        writeStmts(ast.stmtList.get());
    });
}

void AstPrinter::visit(AstReturnStmt& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeExpr(ast.expr.get());
    });
}

//----------------------------------------
// Type (user defined)
//----------------------------------------

void AstPrinter::visit(AstTypeDecl& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeAttributes(ast.attributes.get());
        m_json.attribute("id", ast.name);
        m_json.attributeArray("members", [&] {
            for (const auto& decl : ast.decls) {
                visit(*decl);
            }
        });
    });
}

//----------------------------------------
// IF statement
//----------------------------------------

void AstPrinter::visit(AstIfStmt& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        m_json.attributeArray("blocks", [&] {
            for (const auto& block : ast.blocks) {
                m_json.object([&] {
                    if (!block.decls.empty()) {
                        m_json.attributeArray("decls", [&] {
                            for (const auto& decl : block.decls) {
                                visit(*decl);
                            }
                        });
                    }

                    writeExpr(block.expr.get());

                    if (auto* list = dyn_cast<AstStmtList>(block.stmt.get())) {
                        writeStmts(list);
                    } else {
                        m_json.attributeBegin("stmt");
                        visit(*block.stmt);
                        m_json.attributeEnd();
                    }
                });
            }
        });
    });
}

void AstPrinter::visit(AstForStmt& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        if (!ast.decls.empty()) {
            m_json.attributeArray("decls", [&] {
                for (const auto& decl : ast.decls) {
                    visit(*decl);
                }
            });
        }

        m_json.attributeBegin("iter");
        visit(*ast.iterator);
        m_json.attributeEnd();
        writeExpr(ast.limit.get(), "limit");
        writeExpr(ast.step.get(), "step");

        if (auto* list = dyn_cast<AstStmtList>(ast.stmt.get())) {
            writeStmts(list);
        } else {
            m_json.attributeBegin("stmt");
            visit(*ast.stmt);
            m_json.attributeEnd();
        }

        if (!ast.next.empty()) {
            m_json.attribute("next", ast.next);
        }
    });
}

void AstPrinter::visit(AstDoLoopStmt& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);

        switch (ast.condition) {
        case AstDoLoopStmt::Condition::None:
            break;
        case AstDoLoopStmt::Condition::PreWhile:
            m_json.attribute("makeCondition", "PreWhile");
            break;
        case AstDoLoopStmt::Condition::PreUntil:
            m_json.attribute("makeCondition", "PreUntil");
            break;
        case AstDoLoopStmt::Condition::PostWhile:
            m_json.attribute("makeCondition", "PostWhile");
            break;
        case AstDoLoopStmt::Condition::PostUntil:
            m_json.attribute("makeCondition", "PostUntil");
            break;
        }
        writeExpr(ast.expr.get());

        if (auto* list = dyn_cast<AstStmtList>(ast.stmt.get())) {
            writeStmts(list);
        } else {
            m_json.attributeBegin("stmt");
            visit(*ast.stmt);
            m_json.attributeEnd();
        }
    });
}

void AstPrinter::visit(AstControlFlowBranch& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);

        m_json.attributeBegin("op");
        switch (ast.action) {
        case AstControlFlowBranch::Action::Exit:
            m_json.value("EXIT");
            break;
        case AstControlFlowBranch::Action::Continue:
            m_json.value("CONTINUE");
            break;
        }
        m_json.attributeEnd();

        if (!ast.destination.empty()) {
            m_json.attributeArray("dest", [&] {
                for (auto target : ast.destination) {
                    switch (target) {
                    case ControlFlowStatement::For:
                        m_json.value("FOR");
                        continue;
                    case ControlFlowStatement::Do:
                        m_json.value("DO");
                        continue;
                    }
                }
            });
        }
    });
}

void AstPrinter::visit(AstAttributeList& ast) noexcept {
    m_json.array([&] {
        for (const auto& attr : ast.attribs) {
            visit(*attr);
        }
    });
}

void AstPrinter::visit(AstAttribute& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeIdent(ast.identExpr.get());
        if (!ast.argExprs.empty()) {
            m_json.attributeArray("args", [&] {
                for (const auto& arg : ast.argExprs) {
                    visit(*arg);
                }
            });
        }
    });
}

void AstPrinter::visit(AstTypeExpr& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        m_json.attribute("id", Token::description(ast.tokenKind));
    });
}

void AstPrinter::visit(AstIdentExpr& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        m_json.attribute("id", ast.name);
    });
}

void AstPrinter::visit(AstCallExpr& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeIdent(ast.identExpr.get());
        if (!ast.argExprs.empty()) {
            m_json.attributeArray("args", [&] {
                for (const auto& arg : ast.argExprs) {
                    visit(*arg);
                }
            });
        }
    });
}

void AstPrinter::visit(AstLiteralExpr& ast) noexcept {
    using Ret = std::pair<TokenKind, string>;
    const auto visitor = Visitor{
        [](std::monostate /*value*/) -> Ret {
            return { TokenKind::NullLiteral, "null" };
        },
        [](StringRef value) -> Ret {
            return { TokenKind::StringLiteral, value.str() };
        },
        [&](uint64_t value) -> Ret {
            if (ast.type->isSignedIntegral()) {
                auto sval = static_cast<int64_t>(value);
                return { TokenKind::IntegerLiteral, std::to_string(sval) };
            }
            return { TokenKind::IntegerLiteral, std::to_string(value) };
        },
        [](double value) -> Ret {
            return { TokenKind::FloatingPointLiteral, std::to_string(value) };
        },
        [](bool value) -> Ret {
            return { TokenKind::BooleanLiteral, value ? "TRUE" : "FALSE" };
        }
    };

    m_json.object([&] {
        writeHeader(ast);
        auto [kind, value] = std::visit(visitor, ast.value);
        m_json.attribute("kind", Token::description(kind));
        m_json.attribute("value", value);
        if (ast.type != nullptr) {
            m_json.attribute("type", ast.type->asString());
        }
    });
}

void AstPrinter::visit(AstUnaryExpr& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        m_json.attribute("op", Token::description(ast.tokenKind));
        writeExpr(ast.expr.get());
    });
}

void AstPrinter::visit(AstDereference& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeExpr(&ast);
    });
}

void AstPrinter::visit(AstAddressOf& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeExpr(ast.expr.get());
    });
}

void AstPrinter::visit(AstBinaryExpr& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        m_json.attribute("op", Token::description(ast.tokenKind));
        writeExpr(ast.lhs.get(), "lhs");
        writeExpr(ast.rhs.get(), "rhs");
    });
}

void AstPrinter::visit(AstCastExpr& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        m_json.attribute("implicit", ast.implicit);
        writeType(ast.typeExpr.get());
        writeExpr(ast.expr.get());
    });
}

void AstPrinter::visit(AstIfExpr& ast) noexcept {
    m_json.object([&] {
        writeHeader(ast);
        writeExpr(ast.expr.get(), "expr");
        writeExpr(ast.trueExpr.get(), "true");
        writeExpr(ast.falseExpr.get(), "false");
    });
}

void AstPrinter::writeHeader(AstRoot& ast) noexcept {
    m_json.attribute("class", ast.getClassName());
    m_json.attributeBegin("loc");
    writeLocation(ast);
    m_json.attributeEnd();
}

void AstPrinter::writeLocation(AstRoot& ast) noexcept {
    auto [startLine, startCol] = m_context.getSourceMrg().getLineAndColumn(ast.range.Start);
    auto [endLine, endCol] = m_context.getSourceMrg().getLineAndColumn(ast.range.End);

    if (startLine == endLine) {
        m_json.value(llvm::formatv("{0}:{1} - {2}", startLine, startCol, endCol));
    } else {
        m_json.value(llvm::formatv("{0}:{1} - {2}:3", startLine, startCol, endLine, endCol));
    }
}

void AstPrinter::writeAttributes(AstAttributeList* ast) noexcept {
    if (ast == nullptr || ast->attribs.empty()) {
        return;
    }
    m_json.attributeBegin("attrs");
    visit(*ast);
    m_json.attributeEnd();
}

void AstPrinter::writeStmts(AstStmtList* ast) noexcept {
    if (ast == nullptr || ast->stmts.empty()) {
        return;
    }
    m_json.attributeBegin("stmts");
    visit(*ast);
    m_json.attributeEnd();
}

void AstPrinter::writeExpr(AstExpr* ast, StringRef name) noexcept {
    if (ast == nullptr) {
        return;
    }
    m_json.attributeBegin(name);
    visit(*ast);
    m_json.attributeEnd();
}

void AstPrinter::writeIdent(AstIdentExpr* ast) noexcept {
    if (ast == nullptr) {
        return;
    }
    m_json.attributeBegin("ident");
    visit(*ast);
    m_json.attributeEnd();
}

void AstPrinter::writeType(AstTypeExpr* ast) noexcept {
    if (ast == nullptr) {
        return;
    }
    m_json.attributeBegin("type");
    visit(*ast);
    m_json.attributeEnd();
}
