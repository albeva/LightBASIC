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
    constexpr auto visitor = Visitor{
        [](std::monostate) -> string {
            return "NULL";
        },
        [](const StringRef& value) -> string {
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
        [](uint64_t value) -> string {
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
    m_os << "CAST(";
    if (ast->implicit) {
        m_os << "ANY /' implicit '/";
    } else {
        visit(ast->typeExpr.get());
    }
    m_os << ", ";
    visit(ast->expr.get());
    m_os << ")";
}

string CodePrinter::indent() const noexcept {
    return string(m_indent * SPACES, ' ');
}
