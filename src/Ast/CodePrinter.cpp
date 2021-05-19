////
//// Created by Albert Varaksin on 05/07/2020.
////
#include "CodePrinter.h"
#include "Ast.h"
#include "Lexer/Token.h"
#include "Type/Type.h"
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
    m_os << indent();
    visit(ast->identExpr.get());
    m_os << " = ";
    visit(ast->expr.get());
}

void CodePrinter::visit(AstExprStmt* ast) noexcept {
    m_os << indent();
    visit(ast->expr.get());
}

// Attributes

void CodePrinter::visit(AstAttributeList* ast) noexcept {
    m_os << indent();
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

    m_os << indent();
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

    m_os << indent();

    if (!ast->hasImpl) {
        m_os << "DECLARE ";
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

void CodePrinter::visit(AstFuncStmt* ast) noexcept {
    visit(ast->decl.get());
    m_os << '\n';
    m_indent++;
    visit(ast->stmtList.get());
    m_indent--;

    m_os << indent();
    m_os << "END " << (ast->decl->retTypeExpr ? "FUNCTION" : "SUB");
}

void CodePrinter::visit(AstReturnStmt* ast) noexcept {
    m_os << indent() << "RETURN";
    if (ast->expr) {
        m_os << " ";
        visit(ast->expr.get());
    }
}

void CodePrinter::visit(AstIfStmt* ast) noexcept {
    bool isFirst = true;
    for (const auto& block : ast->blocks) {
        m_os << indent();
        if (!isFirst) {
            m_os << "ELSE";
        }
        if (block.expr) {
            RESTORE_ON_EXIT(m_indent);
            m_indent = 0;
            if (!isFirst) {
                m_os << " ";
            }
            m_os << "IF ";
            for (const auto& var : block.decls) {
                visit(var.get());
                m_os << ", ";
            }
            visit(block.expr.get());
            m_os << " THEN\n";
        } else {
            m_os << "\n";
        }
        m_indent++;
        visit(block.stmt.get());
        if (block.stmt->kind() != AstKind::StmtList) {
            m_os << '\n';
        }
        m_indent--;
        isFirst = false;
    }
    m_os << indent() << "END IF";
}

void CodePrinter::visit(AstForStmt* ast) noexcept {
    m_os << indent() << "FOR ";

    for (const auto& decl : ast->decls) {
        visit(decl.get());
        m_os << ", ";
    }

    m_os << ast->iterator->id;
    if (ast->iterator->typeExpr) {
        m_os << " AS ";
        visit(ast->iterator->typeExpr.get());
    }

    m_os << " = ";
    visit(ast->iterator->expr.get());
    m_os << " TO ";
    visit(ast->limit.get());
    if (ast->step) {
        m_os << " STEP ";
        visit(ast->step.get());
    }

    if (ast->stmt->kind() == AstKind::StmtList) {
        m_os << '\n';
        m_indent++;
        visit(ast->stmt.get());
        m_indent--;
        m_os << "NEXT";
        if (!ast->next.empty()) {
            m_os << " " << ast->next;
        }
    } else {
        m_os << " DO ";
        visit(ast->stmt.get());
    }
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

void CodePrinter::visit(AstLiteralExpr* ast) noexcept {
    const auto visitor = Visitor{
        [](std::monostate /*value*/) -> string {
            return "NULL";
        },
        [](StringRef value) -> string {
            constexpr std::array chars{
                std::make_pair('\"', "\\\""),
                std::make_pair('\n', "\\n"),
                std::make_pair('\t', "\\t")
            };
            string s = value.str();
            for (const auto& r : chars) {
                std::string::size_type pos = 0u;
                while ((pos = s.find(r.first, pos)) != std::string::npos) {
                    s.replace(pos, 1, r.second);
                    pos += 2;
                }
            }
            return '"' + s + '"';
        },
        [&](uint64_t value) -> string {
            if (ast->type->isSignedIntegral()) {
                auto sval = static_cast<int64_t>(value);
                return std::to_string(sval);
            }
            return std::to_string(value);
        },
        [](double value) -> string {
            return std::to_string(value);
        },
        [](bool value) -> string {
            return value ? "TRUE" : "FALSE";
        }
    };

    m_os << std::visit(visitor, ast->value);
}

void CodePrinter::visit(AstUnaryExpr* ast) noexcept {
    m_os << "(";
    auto tkn = Token::create(ast->tokenKind, ast->getRange());
    if (tkn->isRightToLeft()) {
        visit(ast->expr.get());
        m_os << " " << tkn->description();
    } else {
        m_os << tkn->description() << " ";
        visit(ast->expr.get());
    }
    m_os << ")";
}

void CodePrinter::visit(AstBinaryExpr* ast) noexcept {
    m_os << "(";
    visit(ast->lhs.get());

    auto tkn = Token::create(ast->tokenKind, ast->getRange());
    m_os << " " << tkn->description() << " ";

    visit(ast->rhs.get());
    m_os << ")";
}

void CodePrinter::visit(AstCastExpr* ast) noexcept {
    m_os << "(";
    visit(ast->expr.get());
    m_os << " AS ";
    if (ast->implicit) {
        if (ast->type) {
            m_os << ast->type->asString();
        } else {
            m_os << "ANY";
        }
        m_os << " /' implicit '/";
    } else {
        visit(ast->typeExpr.get());
    }
    m_os << ")";
}

void CodePrinter::visit(AstIfExpr* ast) noexcept {
    m_os << "(IF ";
    visit(ast->expr.get());
    m_os << " THEN ";
    visit(ast->trueExpr.get());
    m_os << " ELSE ";
    visit(ast->falseExpr.get());
    m_os << ")";
}

string CodePrinter::indent() const noexcept {
    return string(m_indent * SPACES, ' ');
}
