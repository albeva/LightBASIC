//
// Created by Albert Varaksin on 08/07/2020.
//
#include "SemanticAnalyzer.h"
#include "Ast/Ast.h"
#include "Lexer/Token.h"
#include "Passes/FuncDeclarerPass.h"
#include "Symbol/Symbol.h"
#include "Symbol/SymbolTable.h"
#include "Type/Type.h"
#include <charconv>
using namespace lbc;

SemanticAnalyzer::SemanticAnalyzer(Context& context) noexcept
: m_context{ context },
  m_constantFolder{ context } {}

void SemanticAnalyzer::visit(AstModule* ast) noexcept {
    m_astRootModule = ast;
    m_fileId = ast->fileId;
    ast->symbolTable = make_unique<SymbolTable>(nullptr);

    Sem::FuncDeclarerPass(m_context).visit(ast);

    m_rootTable = m_table = ast->symbolTable.get();
    visit(ast->stmtList.get());
}

void SemanticAnalyzer::visit(AstStmtList* ast) noexcept {
    for (auto& stmt : ast->stmts) {
        visit(stmt.get());
    }
}

void SemanticAnalyzer::visit(AstAssignStmt* ast) noexcept {
    visit(ast->identExpr.get());
    expression(ast->expr, ast->identExpr->type);
}

void SemanticAnalyzer::visit(AstExprStmt* ast) noexcept {
    expression(ast->expr);
}

void SemanticAnalyzer::visit(AstVarDecl* ast) noexcept {
    auto* symbol = createNewSymbol(ast, ast->id);
    symbol->setExternal(false);

    // m_type expr?
    const TypeRoot* type = nullptr;
    if (ast->typeExpr) {
        visit(ast->typeExpr.get());
        type = ast->typeExpr->type;
    }

    // expression?
    if (ast->expr) {
        expression(ast->expr, type);
        if (type == nullptr) {
            type = ast->expr->type;
        }
    }

    // create function symbol
    symbol->setType(type);
    ast->symbol = symbol;

    // alias?
    if (ast->attributes) {
        if (auto alias = ast->attributes->getStringLiteral("ALIAS")) {
            symbol->setAlias(*alias);
        }
    }
}

//----------------------------------------
// Functions
//----------------------------------------

/**
 * Analyze function declaration
 */
void SemanticAnalyzer::visit(AstFuncDecl* /*ast*/) noexcept {
    // NOOP
}

void SemanticAnalyzer::visit(AstFuncParamDecl* /*ast*/) noexcept {
    llvm_unreachable("visitFuncParamDecl");
}

void SemanticAnalyzer::visit(AstFuncStmt* ast) noexcept {
    RESTORE_ON_EXIT(m_table);
    RESTORE_ON_EXIT(m_function);
    m_function = ast->decl.get();
    m_table = ast->decl->symbolTable.get();
    visit(ast->stmtList.get());
}

void SemanticAnalyzer::visit(AstReturnStmt* ast) noexcept {
    const auto* retType = m_function->retTypeExpr->type;
    auto isVoid = isa<TypeVoid>(retType);
    if (!ast->expr) {
        if (!isVoid) {
            fatalError("Expected expression");
        }
        return;
    }

    if (isVoid) {
        fatalError("Unexpected expression for SUB");
    }

    expression(ast->expr);

    if (ast->expr->type != retType) {
        fatalError(
            "Return expression type mismatch."_t
            + " Expected (" + retType->asString() + ")"
            + " got (" + ast->expr->type->asString() + ")");
    }
}

//----------------------------------------
// Attributes
//----------------------------------------

void SemanticAnalyzer::visit(AstAttributeList* /*ast*/) noexcept {
    llvm_unreachable("visitAttributeList");
}

void SemanticAnalyzer::visit(AstAttribute* /*ast*/) noexcept {
    llvm_unreachable("visitAttribute");
}

//----------------------------------------
// Types
//----------------------------------------

void SemanticAnalyzer::visit(AstTypeExpr* ast) noexcept {
    ast->type = TypeRoot::fromTokenKind(ast->tokenKind);
}

//----------------------------------------
// Expressions
//----------------------------------------

void SemanticAnalyzer::expression(unique_ptr<AstExpr>& ast, const TypeRoot* type) noexcept {
    visit(ast.get());
    if (type != nullptr) {
        coerce(ast, type);
    }
    m_constantFolder.fold(ast);
}

void SemanticAnalyzer::visit(AstIdentExpr* ast) noexcept {
    const auto& name = ast->id;
    auto* symbol = m_table->find(name, true);

    if (symbol == nullptr) {
        fatalError("Unknown identifier "_t + name);
    }

    if (symbol->type() == nullptr) {
        fatalError("Identifier "_t + name + " has unresolved m_type");
    }

    ast->symbol = symbol;
    ast->type = symbol->type();
}

void SemanticAnalyzer::visit(AstCallExpr* ast) noexcept {
    visit(ast->identExpr.get());

    auto* symbol = ast->identExpr->symbol;

    const auto* type = dyn_cast<TypeFunction>(symbol->type());
    if (type == nullptr) {
        fatalError("Identifier "_t + symbol->name() + " is not a callable m_type");
    }

    const auto& paramTypes = type->getParams();
    auto& args = ast->argExprs;

    if (type->isVariadic()) {
        if (paramTypes.size() > args.size()) {
            fatalError("Argument count mismatch");
        }
    } else if (paramTypes.size() != args.size()) {
        fatalError("Argument count mismatch");
    }

    for (size_t index = 0; index < args.size(); index++) {
        if (index < paramTypes.size()) {
            expression(args[index], args[index]->type);
        } else {
            expression(args[index]);
        }
    }

    ast->type = type->getReturn();
}

void SemanticAnalyzer::visit(AstLiteralExpr* ast) noexcept {
    constexpr auto visitor = Visitor{
        [](const std::monostate&) {
            return TokenKind::Null;
        },
        [](const StringRef&) {
            return TokenKind::ZString;
        },
        [](uint64_t value) {
            if (value > static_cast<uint64_t>(std::numeric_limits<int32_t>::max())) {
                return TokenKind::Long;
            }
            return TokenKind::Integer;
        },
        [](double) {
            return TokenKind::Double;
        },
        [](bool) {
            return TokenKind::Bool;
        }
    };
    auto typeKind = std::visit(visitor, ast->value);
    ast->type = TypeRoot::fromTokenKind(typeKind);
}

void SemanticAnalyzer::visit(AstUnaryExpr* ast) noexcept {
    expression(ast->expr);
    if (!isa<TypeNumeric>(ast->expr->type)) {
        fatalError("Applying unary - to non numeric type");
    }
    ast->type = ast->expr->type;
}

void SemanticAnalyzer::visit(AstCastExpr* /*ast*/) noexcept {
    fatalError("CAST not implemented");
}

//----------------------------------------
// Utils
//----------------------------------------

Symbol* SemanticAnalyzer::createNewSymbol(AstDecl* ast, const StringRef& id) noexcept {
    if (m_table->find(id, false) != nullptr) {
        fatalError("Redefinition of "_t + id);
    }
    auto* symbol = m_table->insert(id);

    // alias?
    if (ast->attributes) {
        if (auto alias = ast->attributes->getStringLiteral("ALIAS")) {
            symbol->setAlias(*alias);
        }
    }

    return symbol;
}

void SemanticAnalyzer::coerce(unique_ptr<AstExpr>& ast, const TypeRoot* type) noexcept {
    if (ast->type == type) {
        return;
    }

    if (isa<TypeNumeric>(type) && isa<TypeNumeric>(ast->type)) {
        return cast(ast, type);
    }

    fatalError(
        "Type mismatch."_t
        + " Expected '" + type->asString() + "'"
        + " got '" + ast->type->asString() + "'");
}

void SemanticAnalyzer::cast(unique_ptr<AstExpr>& ast, const TypeRoot* type) noexcept {
    auto cast = AstCastExpr::create(ast->getRange());
    cast->expr.swap(ast);
    cast->type = type;
    cast->implicit = true;
    ast.reset(cast.release()); // NOLINT
}
