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
  m_constantFolder{} {}

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
        [](const std::monostate& /*value*/) {
            return TokenKind::Null;
        },
        [](const StringRef& /*value*/) {
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

void SemanticAnalyzer::visit(AstUnaryExpr* ast) noexcept {
    expression(ast->expr);
    const auto* type = ast->expr->type;

    switch (ast->tokenKind) {
    case TokenKind::LogicalNot:
        if (type->isBoolean()) {
            ast->type = type;
            return;
        }
        if (type->isNumeric()) {
            cast(ast->expr, TypeBoolean::get());
            m_constantFolder.fold(ast->expr);
            ast->type = ast->expr->type;
            return;
        }
        fatalError("Applying unary NOT to non-numeric pr bool type");
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

void SemanticAnalyzer::visit(AstBinaryExpr* ast) noexcept {
    expression(ast->lhs);
    expression(ast->rhs);
    const auto* left = ast->lhs->type;
    const auto* right = ast->rhs->type;

    switch (Token::getOperatorType(ast->tokenKind)) {
    case OperatorType::Arithmetic: {
//        auto convert = [&](unique_ptr<AstExpr>& expr, const TypeRoot* ty) {
//            cast(expr, ty);
//            m_constantFolder.fold(expr);
//            ast->type = ty;
//        };
//
//        const auto* lt = dyn_cast<TypeNumeric>(ast->lhs->type);
//        const auto* rt = dyn_cast<TypeNumeric>(ast->rhs->type);
//        if (lt == nullptr || rt == nullptr) {
//            fatalError("Applying binary operation to non numeric type");
//        }
//
//        if (lt == rt) {
//            ast->type = lt;
//            return;
//        }
//
//        if (const auto* li = dyn_cast<TypeIntegral>(lt)) {
//            if (const auto* ri = dyn_cast<TypeIntegral>(rt)) {
//                if (li->getBits() > ri->getBits()) {
//                    return convert(ast->rhs, li);
//                }
//                if (li->getBits() < ri->getBits()) {
//                    return convert(ast->lhs, ri);
//                }
//                if (li->isSigned()) {
//                    return convert(ast->rhs, li);
//                }
//                if (ri->isSigned()) {
//                    return convert(ast->lhs, ri);
//                }
//            } else if (const auto* rfp = dyn_cast<TypeFloatingPoint>(rt)) {
//                return convert(ast->lhs, rfp);
//            }
//        } else if (const auto& lfp = dyn_cast<TypeFloatingPoint>(lt)) {
//            if (const auto* ri = dyn_cast<TypeIntegral>(rt)) {
//                return convert(ast->rhs, lfp);
//            }
//            if (const auto* rfp = dyn_cast<TypeFloatingPoint>(rt)) {
//                if (lfp->getBits() > rfp->getBits()) {
//                    return convert(ast->rhs, lfp);
//                }
//                return convert(ast->lhs, rfp);
//            }
//        }
        llvm_unreachable("Unknown difference between types");
    }
    case OperatorType::Comparison: {
        if (left->isBoolean() && right->isBoolean()) {
            if (ast->tokenKind == TokenKind::Equal || ast->tokenKind == TokenKind::NotEqual) {
                ast->type = left;
                return;
            }
            fatalError("Applying unsupported operation to boolean type");
        }

        if (!left->isNumeric() || !right->isNumeric()) {
            fatalError("Applying binary operation to non numeric type");
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
    case OperatorType::Logical:
        // bool OP bool -> bool
        llvm_unreachable("Logical operators not implemented yet");
        break;
    }
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
    const auto* src = type;
    const auto* dst = ast->type;

    if ((src->isNumeric() || src->isBoolean())
        && (dst->isNumeric() || dst->isBoolean())) {
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
