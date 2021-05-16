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
    auto isVoid = retType->isVoid();
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

void SemanticAnalyzer::visit(AstIfStmt* ast) noexcept {
    RESTORE_ON_EXIT(m_table);
    for (auto& block: ast->blocks) {
        block.symbolTable = make_unique<SymbolTable>(m_table);
    }

    for (size_t idx = 0; idx < ast->blocks.size(); idx++) {
        auto& block = ast->blocks[idx];

        m_table = block.symbolTable.get();
        for (auto& var: block.decls) {
            visit(var.get());
            for (size_t next = idx + 1; next < ast->blocks.size(); next++) {
                ast->blocks[next].symbolTable->addReference(var->symbol);
            }
        }
        if (block.expr) {
            expression(block.expr);
            if (!block.expr->type->isBoolean()) {
                fatalError("type '"_t
                    + block.expr->type->asString()
                    + "' cannot be used as boolean");
            }
        }
        visit(block.stmt.get());
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
    m_constantFolder.fold(ast);
    if (type != nullptr) {
        coerce(ast, type);
        m_constantFolder.fold(ast);
    }
}

void SemanticAnalyzer::visit(AstIdentExpr* ast) noexcept {
    const auto& name = ast->id;
    auto* symbol = m_table->find(name, true);

    if (symbol == nullptr) {
        fatalError("Unknown identifier "_t + name);
    }

    if (symbol->type() == nullptr) {
        fatalError("Identifier "_t + name + " has unresolved type");
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
        [](const std::monostate& /*value*/) {
            return TokenKind::Null;
        },
        [](StringRef /*value*/) {
            return TokenKind::ZString;
        },
        [](uint64_t value) {
            if (value > static_cast<uint64_t>(std::numeric_limits<int32_t>::max())) {
                return TokenKind::Long;
            }
            return TokenKind::Integer;
        },
        [](double /*value*/) {
            return TokenKind::Double;
        },
        [](bool /*value*/) {
            return TokenKind::Bool;
        }
    };
    auto typeKind = std::visit(visitor, ast->value);
    ast->type = TypeRoot::fromTokenKind(typeKind);
}

//------------------------------------------------------------------
// Unary Expressions
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstUnaryExpr* ast) noexcept {
    expression(ast->expr);
    const auto* type = ast->expr->type;

    switch (ast->tokenKind) {
    case TokenKind::LogicalNot:
        if (type->isBoolean()) {
            ast->type = type;
            return;
        }
        fatalError("Applying unary NOT to non bool type");
    case TokenKind::Negate:
        if (type->isNumeric()) {
            ast->type = type;
            return;
        }
        fatalError("Applying unary negate to non-numeric type");
    default:
        llvm_unreachable("unknown unary operator");
    }
}

//------------------------------------------------------------------
// Binary Expressions
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstBinaryExpr* ast) noexcept {
    expression(ast->lhs);
    expression(ast->rhs);

    switch (Token::getOperatorType(ast->tokenKind)) {
    case OperatorType::Arithmetic:
        return arithmetic(ast);
    case OperatorType::Comparison:
        return comparison(ast);
    case OperatorType::Logical:
        return logical(ast);
    }
}

void SemanticAnalyzer::arithmetic(AstBinaryExpr* ast) noexcept {
    const auto* left = ast->lhs->type;
    const auto* right = ast->rhs->type;

    if (!left->isNumeric() || !right->isNumeric()) {
        fatalError("Applying artithmetic operation to non numeric type");
    }

    const auto convert = [&](unique_ptr<AstExpr>& expr, const TypeRoot* ty) noexcept {
        cast(expr, ty);
        m_constantFolder.fold(expr);
        ast->type = ty;
    };

    switch (left->compare(right)) {
    case TypeComparison::Incompatible:
        fatalError("Operator on incompatible types");
    case TypeComparison::Downcast:
        return convert(ast->rhs, left);
    case TypeComparison::Equal:
        ast->type = left;
        return;
    case TypeComparison::Upcast:
        return convert(ast->lhs, right);
    }
}

void SemanticAnalyzer::logical(AstBinaryExpr* ast) noexcept {
    const auto* left = ast->lhs->type;
    const auto* right = ast->rhs->type;

    if (!left->isBoolean() || !right->isBoolean()) {
        fatalError("Applying logical operator to non boolean type");
    }
    ast->type = left;
}

void SemanticAnalyzer::comparison(AstBinaryExpr* ast) noexcept {
    const auto* left = ast->lhs->type;
    const auto* right = ast->rhs->type;

    if (left->isBoolean() && right->isBoolean()) {
        if (ast->tokenKind == TokenKind::Equal || ast->tokenKind == TokenKind::NotEqual) {
            ast->type = left;
            return;
        }
        fatalError("Applying unsupported operation to boolean type");
    }

    if (!left->isNumeric() || !right->isNumeric()) {
        fatalError("Applying comparison operation to non numeric type");
    }

    const auto convert = [&](unique_ptr<AstExpr>& expr, const TypeRoot* ty) noexcept {
        cast(expr, ty);
        m_constantFolder.fold(expr);
        ast->type = TypeBoolean::get();
    };

    switch (left->compare(right)) {
    case TypeComparison::Incompatible:
        fatalError("Operator on incompatible types");
    case TypeComparison::Downcast:
        return convert(ast->rhs, left);
    case TypeComparison::Equal:
        ast->type = TypeBoolean::get();
        return;
    case TypeComparison::Upcast:
        return convert(ast->lhs, right);
    }
}

//------------------------------------------------------------------
// Casting
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstCastExpr* /*ast*/) noexcept {
    fatalError("CAST not implemented");
}

void SemanticAnalyzer::coerce(unique_ptr<AstExpr>& ast, const TypeRoot* type) noexcept {
    if (ast->type == type) {
        return;
    }
    const auto* src = type;
    const auto* dst = ast->type;

    switch (src->compare(dst)) {
    case TypeComparison::Incompatible:
        fatalError(
            "Type mismatch."_t
            + " Expected '" + type->asString() + "'"
            + " got '" + ast->type->asString() + "'");
    case TypeComparison::Downcast:
    case TypeComparison::Upcast:
        return cast(ast, type);
        break;
    case TypeComparison::Equal:
        return;
    }
}

void SemanticAnalyzer::cast(unique_ptr<AstExpr>& ast, const TypeRoot* type) noexcept {
    auto cast = AstCastExpr::create(ast->getRange());
    cast->expr.swap(ast);
    cast->type = type;
    cast->implicit = true;
    ast.reset(cast.release()); // NOLINT
}

//------------------------------------------------------------------
// IfExpr
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstIfExpr* ast) noexcept {
    expression(ast->expr, TypeBoolean::get());
    expression(ast->trueExpr);
    expression(ast->falseExpr);

    const auto convert = [&](unique_ptr<AstExpr>& expr, const TypeRoot* ty) noexcept {
        cast(expr, ty);
        m_constantFolder.fold(expr);
        ast->type = ty;
    };

    const auto* left = ast->trueExpr->type;
    const auto* right = ast->falseExpr->type;
    switch (left->compare(right)) {
    case TypeComparison::Incompatible:
        fatalError("Incompatible types");
    case TypeComparison::Downcast:
        return convert(ast->falseExpr, left);
    case TypeComparison::Equal:
        ast->type = left;
        return;
    case TypeComparison::Upcast:
        return convert(ast->trueExpr, right);
    }
}

//------------------------------------------------------------------
// Utils
//------------------------------------------------------------------

Symbol* SemanticAnalyzer::createNewSymbol(AstDecl* ast, StringRef id) noexcept {
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
