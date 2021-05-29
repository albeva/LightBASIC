////
//// Created by Albert Varaksin on 05/07/2020.
////
#include "CodePrinter.hpp"
#include "Ast.hpp"
#include "Lexer/Token.hpp"
#include "Type/Type.hpp"
using namespace lbc;

void CodePrinter::visit(AstModule& ast) noexcept {
    visit(*ast.stmtList);
}

// Statements

void CodePrinter::visit(AstStmtList& ast) noexcept {
    for (const auto& stmt : ast.stmts) {
        visit(*stmt);
        m_os << '\n';
    }
}

void CodePrinter::visit(AstAssignStmt& ast) noexcept {
    m_os << indent();
    visit(*ast.lhs);
    m_os << " = ";
    visit(*ast.rhs);
}

void CodePrinter::visit(AstExprStmt& ast) noexcept {
    m_os << indent();
    visit(*ast.expr);
}

// Attributes

void CodePrinter::visit(AstAttributeList& ast) noexcept {
    m_os << indent();
    m_os << '[';
    bool isFirst = true;
    for (const auto& attr : ast.attribs) {
        if (isFirst) {
            isFirst = false;
        } else {
            m_os << ", ";
        }
        visit(*attr);
    }
    m_os << "]";
}

void CodePrinter::visit(AstAttribute& ast) noexcept {
    visit(*ast.identExpr);
    if (ast.argExprs.size() == 1) {
        m_os << " = ";
        visit(*ast.argExprs[0]);
    } else if (ast.argExprs.size() > 1) {
        bool isFirst = true;
        m_os << "(";
        for (const auto& arg : ast.argExprs) {
            if (isFirst) {
                isFirst = false;
            } else {
                m_os << ", ";
            }
            visit(*arg);
        }
        m_os << ")";
    }
}

void CodePrinter::visit(AstTypeExpr& ast) noexcept {
    m_os << Token::description(ast.tokenKind);
}

// Declarations

void CodePrinter::visit(AstVarDecl& ast) noexcept {
    if (ast.attributes) {
        visit(*ast.attributes);
        m_os << " _" << '\n';
    }

    m_os << indent();
    if (emitVARkeyword) {
        m_os << "VAR ";
    }
    m_os << ast.name;

    if (ast.typeExpr) {
        m_os << " AS ";
        visit(*ast.typeExpr);
    }

    if (ast.expr) {
        m_os << " = ";
        visit(*ast.expr);
    }
}

void CodePrinter::visit(AstFuncDecl& ast) noexcept {
    if (ast.attributes) {
        visit(*ast.attributes);
        m_os << " _" << '\n';
    }

    m_os << indent();

    if (!ast.hasImpl) {
        m_os << "DECLARE ";
    }

    if (ast.retTypeExpr) {
        m_os << "FUNCTION ";
    } else {
        m_os << "SUB ";
    }
    m_os << ast.name;

    if (!ast.paramDecls.empty()) {
        m_os << "(";
        bool isFirst = true;
        for (const auto& param : ast.paramDecls) {
            if (isFirst) {
                isFirst = false;
            } else {
                m_os << ", ";
            }
            visit(*param);
        }
        m_os << ")";
    }

    if (ast.retTypeExpr) {
        m_os << " AS ";
        visit(*ast.retTypeExpr);
    }
}

void CodePrinter::visit(AstFuncParamDecl& ast) noexcept {
    m_os << ast.name;
    m_os << " AS ";
    visit(*ast.typeExpr);
}

void CodePrinter::visit(AstFuncStmt& ast) noexcept {
    visit(*ast.decl);
    m_os << '\n';
    m_indent++;
    visit(*ast.stmtList);
    m_indent--;

    m_os << indent();
    m_os << "END " << (ast.decl->retTypeExpr ? "FUNCTION" : "SUB");
}

void CodePrinter::visit(AstReturnStmt& ast) noexcept {
    m_os << indent() << "RETURN";
    if (ast.expr) {
        m_os << " ";
        visit(*ast.expr);
    }
}

//----------------------------------------
// Type (user defined)
//----------------------------------------

void CodePrinter::visit(AstTypeDecl& ast) noexcept {
    RESTORE_ON_EXIT(emitVARkeyword);
    emitVARkeyword = false;

    if (ast.attributes) {
        visit(*ast.attributes);
        m_os << " _" << '\n';
    }

    m_os << indent() << "TYPE " << ast.name << '\n';
    m_indent++;
    for (const auto& decl: ast.decls) {
        visit(*decl);
        m_os << '\n';
    }
    m_indent--;
    m_os << indent() << "END TYPE";
}

//----------------------------------------
// IF statement
//----------------------------------------

void CodePrinter::visit(AstIfStmt& ast) noexcept {
    bool isFirst = true;
    for (const auto& block : ast.blocks) {
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
                visit(*var);
                m_os << ", ";
            }
            visit(*block.expr);
            m_os << " THEN\n";
        } else {
            m_os << "\n";
        }
        m_indent++;
        visit(*block.stmt);
        if (block.stmt->kind != AstKind::StmtList) {
            m_os << '\n';
        }
        m_indent--;
        isFirst = false;
    }
    m_os << indent() << "END IF";
}

void CodePrinter::visit(AstForStmt& ast) noexcept {
    m_os << indent() << "FOR ";

    for (const auto& decl : ast.decls) {
        visit(*decl);
        m_os << ", ";
    }

    m_os << ast.iterator->name;
    if (ast.iterator->typeExpr) {
        m_os << " AS ";
        visit(*ast.iterator->typeExpr);
    }

    m_os << " = ";
    visit(*ast.iterator->expr);
    m_os << " TO ";
    visit(*ast.limit);
    if (ast.step) {
        m_os << " STEP ";
        visit(*ast.step);
    }

    if (ast.stmt->kind == AstKind::StmtList) {
        m_os << '\n';
        m_indent++;
        visit(*ast.stmt);
        m_indent--;
        m_os << indent() << "NEXT";
        if (!ast.next.empty()) {
            m_os << " " << ast.next;
        }
    } else {
        m_os << " DO ";
        visit(*ast.stmt);
    }
}

void CodePrinter::visit(AstDoLoopStmt& ast) noexcept {
    m_os << indent() << "DO";

    if (!ast.decls.empty()) {
        bool first = true;
        for (const auto& decl : ast.decls) {
            if (first) {
                first = false;
                m_os << " ";
            } else {
                m_os << ", ";
            }
            visit(*decl);
        }
    }

    if (ast.condition == AstDoLoopStmt::Condition::PreWhile) {
        m_os << " WHILE ";
        visit(*ast.expr);
    } else if (ast.condition == AstDoLoopStmt::Condition::PreUntil) {
        m_os << " UNTIL ";
        visit(*ast.expr);
    }

    if (ast.stmt->kind == AstKind::StmtList) {
        m_os << "\n";
        m_indent++;
        visit(*ast.stmt);
        m_indent--;
        m_os << indent() << "LOOP";

        if (ast.condition == AstDoLoopStmt::Condition::PostWhile) {
            m_os << " WHILE ";
            visit(*ast.expr);
        } else if (ast.condition == AstDoLoopStmt::Condition::PostUntil) {
            m_os << " UNTIL ";
            visit(*ast.expr);
        }
    } else {
        m_os << " DO ";
        visit(*ast.stmt);
    }
}

void CodePrinter::visit(AstControlFlowBranch& ast) noexcept {
    m_os << indent();
    switch (ast.action) {
    case AstControlFlowBranch::Action::Continue:
        m_os << "CONTINUE";
        break;
    case AstControlFlowBranch::Action::Exit:
        m_os << "EXIT";
        break;
    }

    if (!ast.destination.empty()) {
        for (auto target : ast.destination) {
            switch (target) {
            case ControlFlowStatement::For:
                m_os << " FOR";
                continue;
            case ControlFlowStatement::Do:
                m_os << " DO";
                continue;
            }
        }
    }
}

// Expressions

void CodePrinter::visit(AstIdentExpr& ast) noexcept {
    m_os << ast.name;
}

void CodePrinter::visit(AstCallExpr& ast) noexcept {
    visit(*ast.identExpr);
    m_os << "(";
    bool isFirst = true;
    for (const auto& arg : ast.argExprs) {
        if (isFirst) {
            isFirst = false;
        } else {
            m_os << ", ";
        }

        visit(*arg);
    }
    m_os << ")";
}

void CodePrinter::visit(AstLiteralExpr& ast) noexcept {
    const auto visitor = Visitor{
        [](std::monostate /*value*/) -> string {
            return "NULL";
        },
        [](StringRef value) -> string {
            string result;
            llvm::raw_string_ostream str{ result };
            llvm::printEscapedString(value, str);
            return '"' + result + '"';
        },
        [&](uint64_t value) -> string {
            if (ast.type->isSignedIntegral()) {
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

    m_os << std::visit(visitor, ast.value);
}

void CodePrinter::visit(AstUnaryExpr& ast) noexcept {
    m_os << "(";
    auto tkn = Token::create(ast.tokenKind, ast.range);
    if (tkn->isRightToLeft()) {
        visit(*ast.expr);
        m_os << " " << tkn->description();
    } else {
        m_os << tkn->description() << " ";
        visit(*ast.expr);
    }
    m_os << ")";
}

void CodePrinter::visit(AstDereference& ast) noexcept {
    m_os << "*(";
    visit(*ast.expr);
    m_os << ")";
}

void CodePrinter::visit(AstAddressOf& ast) noexcept {
    m_os << "@(";
    visit(*ast.expr);
    m_os << ")";
}

void CodePrinter::visit(AstBinaryExpr& ast) noexcept {
    m_os << "(";
    visit(*ast.lhs);

    auto tkn = Token::create(ast.tokenKind, ast.range);
    m_os << " " << tkn->description() << " ";

    visit(*ast.rhs);
    m_os << ")";
}

void CodePrinter::visit(AstCastExpr& ast) noexcept {
    m_os << "(";
    visit(*ast.expr);
    m_os << " AS ";
    if (ast.implicit) {
        if (ast.type != nullptr) {
            m_os << ast.type->asString();
        } else {
            m_os << "ANY";
        }
        m_os << " /' implicit '/";
    } else {
        visit(*ast.typeExpr);
    }
    m_os << ")";
}

void CodePrinter::visit(AstIfExpr& ast) noexcept {
    m_os << "(IF ";
    visit(*ast.expr);
    m_os << " THEN ";
    visit(*ast.trueExpr);
    m_os << " ELSE ";
    visit(*ast.falseExpr);
    m_os << ")";
}

string CodePrinter::indent() const noexcept {
    return string(m_indent * SPACES, ' ');
}
