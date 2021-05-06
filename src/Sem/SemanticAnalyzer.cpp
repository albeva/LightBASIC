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

void SemanticAnalyzer::visit(AstModule* ast) {
    m_astRootModule = ast;
    m_fileId = ast->fileId;
    ast->symbolTable = make_unique<SymbolTable>(nullptr);

    Sem::FuncDeclarerPass(m_context).visit(ast);

    m_rootTable = m_table = ast->symbolTable.get();
    visitStmtList(ast->stmtList.get());
}

void SemanticAnalyzer::visitStmtList(AstStmtList* ast) {
    for (auto& stmt : ast->stmts) {
        visitStmt(stmt.get());
    }
}

void SemanticAnalyzer::visitAssignStmt(AstAssignStmt* ast) {
    visitIdentExpr(ast->identExpr.get());
    expression(ast->expr, ast->identExpr->type);
}

void SemanticAnalyzer::visitExprStmt(AstExprStmt* ast) {
    expression(ast->expr);
}

void SemanticAnalyzer::visitVarDecl(AstVarDecl* ast) {
    auto* symbol = createNewSymbol(ast, ast->token.get());
    symbol->setExternal(false);

    // m_type expr?
    const TypeRoot* type = nullptr;
    if (ast->typeExpr) {
        visitTypeExpr(ast->typeExpr.get());
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
        if (const auto* token = ast->attributes->getStringLiteral("ALIAS")) {
            symbol->setAlias(token->lexeme());
        }
    }
}

//----------------------------------------
// Functions
//----------------------------------------

/**
 * Analyze function declaration
 */
void SemanticAnalyzer::visitFuncDecl(AstFuncDecl* /*ast*/) {
    // NOOP
}

void SemanticAnalyzer::visitFuncParamDecl(AstFuncParamDecl* /*ast*/) {
    llvm_unreachable("visitFuncParamDecl");
}

void SemanticAnalyzer::visitFuncStmt(AstFuncStmt* ast) {
    RESTORE_ON_EXIT(m_table);
    RESTORE_ON_EXIT(m_function);
    m_function = ast->decl.get();
    m_table = ast->decl->symbolTable.get();
    visitStmtList(ast->stmtList.get());
}

void SemanticAnalyzer::visitReturnStmt(AstReturnStmt* ast) {
    auto retType = m_function->retTypeExpr->type;
    auto isVoid = isa<TypeVoid>(retType);
    if (!ast->expr) {
        if (!isVoid) {
            fatalError("Expected expression");
        }
        return;
    } else if (isVoid) {
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

void SemanticAnalyzer::visitAttributeList(AstAttributeList* /*ast*/) {
    llvm_unreachable("visitAttributeList");
}

void SemanticAnalyzer::visitAttribute(AstAttribute* /*ast*/) {
    llvm_unreachable("visitAttribute");
}

//----------------------------------------
// Types
//----------------------------------------

void SemanticAnalyzer::visitTypeExpr(AstTypeExpr* ast) {
    ast->type = TypeRoot::fromTokenKind(ast->token->kind());
}

//----------------------------------------
// Expressions
//----------------------------------------

void SemanticAnalyzer::expression(unique_ptr<AstExpr>& ast, const TypeRoot* type) noexcept {
    visitExpr(ast.get());
    if (type != nullptr) {
        coerce(ast, type);
    }
    m_constantFolder.fold(ast);
}

void SemanticAnalyzer::visitIdentExpr(AstIdentExpr* ast) {
    const auto& name = ast->token->lexeme();
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

void SemanticAnalyzer::visitCallExpr(AstCallExpr* ast) {
    visitIdentExpr(ast->identExpr.get());

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

void SemanticAnalyzer::visitLiteralExpr(AstLiteralExpr* ast) {
    switch (ast->token->kind()) {
    case TokenKind::StringLiteral:
        ast->type = TypeZString::get();
        break;
    case TokenKind::IntegerLiteral: {
        ast->type = TypeRoot::fromTokenKind(TokenKind::ULong);
        uint64_t value;
        const auto& lexeme = ast->token->lexeme();
        if (std::from_chars(lexeme.begin(), lexeme.end(), value, 10).ec != std::errc()) {
            fatalError("Failed to parse number: "_t + lexeme);
        }
        ast->value.uint64 = value;
        break;
    }
    case TokenKind::FloatingPointLiteral: {
        ast->type = TypeRoot::fromTokenKind(TokenKind::Double);
        const auto& lexeme = ast->token->lexeme();
        char* end;
        double value = std::strtod(lexeme.begin(), &end);
        if (end == lexeme.begin()) {
            fatalError("Failed to parse number: "_t + lexeme);
        }
        ast->value.dbl = value;
        break;
    }
    case TokenKind::BooleanLiteral: {
        ast->type = TypeBoolean::get();
        const auto& lexeme = ast->token->lexeme();
        if (lexeme == "TRUE") {
            ast->value.b = true;
        } else if (lexeme == "FALSE") {
            ast->value.b = false;
        } else {
            fatalError("Failed to parse boolean: "_t + lexeme);
        }
        break;
    }
    case TokenKind::NullLiteral:
        ast->type = TypePointer::get(TypeAny::get());
        break;
    default:
        fatalError("Unsupported literal type");
    }
}

void SemanticAnalyzer::visitUnaryExpr(AstUnaryExpr* ast) {
    expression(ast->expr);
    if (!isa<TypeNumeric>(ast->expr->type)) {
        fatalError("Applying unary - to non numeric type");
    }
    ast->type = ast->expr->type;
}

void SemanticAnalyzer::visitCastExpr(AstCastExpr* /*ast*/) {
    fatalError("CAST not implemented");
}

//----------------------------------------
// Utils
//----------------------------------------

Symbol* SemanticAnalyzer::createNewSymbol(AstDecl* ast, Token* token) noexcept {
    if (m_table->find(token->lexeme(), false) != nullptr) {
        fatalError("Redefinition of "_t + token->lexeme());
    }
    auto* symbol = m_table->insert(token->lexeme());

    // alias?
    if (ast->attributes) {
        if (const auto* alias = ast->attributes->getStringLiteral("ALIAS")) {
            symbol->setAlias(alias->lexeme());
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
    auto cast = AstCastExpr::create();
    cast->expr.swap(ast);
    cast->type = type;
    cast->implicit = true;
    ast.reset(cast.release()); // NOLINT
}

