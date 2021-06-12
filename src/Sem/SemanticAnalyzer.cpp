//
// Created by Albert Varaksin on 08/07/2020.
//
#include "SemanticAnalyzer.hpp"
#include "Ast/Ast.hpp"
#include "Driver/Context.hpp"
#include "Lexer/Token.hpp"
#include "Passes/ForStmtPass.hpp"
#include "Passes/FuncDeclarerPass.hpp"
#include "Passes/TypeDeclPass.hpp"
#include "Symbol/Symbol.hpp"
#include "Symbol/SymbolTable.hpp"
#include "Type/Type.hpp"
#include "Type/TypeUdt.hpp"

using namespace lbc;

SemanticAnalyzer::SemanticAnalyzer(Context& context)
: m_context{ context },
  m_constantFolder{ context },
  m_typePass{ *this } {}

void SemanticAnalyzer::visit(AstModule& ast) {
    m_astRootModule = &ast;
    m_fileId = ast.fileId;
    ast.symbolTable = m_context.create<SymbolTable>(nullptr);
    m_rootTable = m_table = ast.symbolTable;

    Sem::FuncDeclarerPass(m_context, m_typePass).visit(ast);

    visit(*ast.stmtList);
}

void SemanticAnalyzer::visit(AstStmtList& ast) {
    for (auto& stmt : ast.stmts) {
        visit(*stmt);
    }
}

void SemanticAnalyzer::visit(AstImport& ast) {
    if (!ast.module) {
        return;
    }

    RESTORE_ON_EXIT(m_fileId);
    m_fileId = ast.module->fileId;
    visit(*ast.module->stmtList);
}

void SemanticAnalyzer::visit(AstExprStmt& ast) {
    expression(ast.expr);
}

void SemanticAnalyzer::visit(AstVarDecl& ast) {
    // m_type expr?
    const TypeRoot* type = nullptr;
    if (ast.typeExpr) {
        visit(*ast.typeExpr);
        type = ast.typeExpr->type;
    }

    // expression?
    if (ast.expr) {
        expression(ast.expr, type);
    }

    if (type == nullptr) {
        type = ast.expr->type;
    }

    // The Symbol
    auto* symbol = createNewSymbol(ast);
    symbol->setExternal(false);

    // create function symbol
    symbol->setType(type);
    ast.symbol = symbol;
    auto flags = ast.symbol->getFlags();
    flags.addressable = true;
    flags.assignable = true;
    if (type->isPointer()) {
        flags.dereferencable = true;
    }
    ast.symbol->setFlags(flags);

    // alias?
    if (ast.attributes) {
        if (auto alias = ast.attributes->getStringLiteral("ALIAS")) {
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
void SemanticAnalyzer::visit(AstFuncDecl& /*ast*/) {
    // NOOP
}

void SemanticAnalyzer::visit(AstFuncParamDecl& /*ast*/) {
    llvm_unreachable("visitFuncParamDecl");
}

void SemanticAnalyzer::visit(AstFuncStmt& ast) {
    RESTORE_ON_EXIT(m_table);
    RESTORE_ON_EXIT(m_function);
    m_function = ast.decl;
    m_table = ast.decl->symbolTable;
    visit(*ast.stmtList);
}

void SemanticAnalyzer::visit(AstReturnStmt& ast) {
    const TypeRoot* retType = nullptr;
    bool canOmitExpression = false;
    if (m_function == nullptr) {
        if (!m_astRootModule->hasImplicitMain) {
            fatalError("Return statement outside SUB / FUNCTION or a main module");
        }
        retType = TypeIntegral::fromTokenKind(TokenKind::Integer);
        canOmitExpression = true;
    } else {
        retType = llvm::cast<TypeFunction>(m_function->symbol->type())->getReturn();
    }
    auto isVoid = retType->isVoid();
    if (!ast.expr) {
        if (!isVoid && !canOmitExpression) {
            fatalError("Expected expression");
        }
        return;
    }

    if (isVoid) {
        fatalError("Unexpected expression for SUB");
    }

    expression(ast.expr);

    if (ast.expr->type != retType) {
        fatalError(
            "Return expression type mismatch."_t
            + " Expected (" + retType->asString() + ")"
            + " got (" + ast.expr->type->asString() + ")");
    }
}

void SemanticAnalyzer::visit(AstIfStmt& ast) {
    RESTORE_ON_EXIT(m_table);
    for (auto& block : ast.blocks) {
        block.symbolTable = m_context.create<SymbolTable>(m_table);
    }

    for (size_t idx = 0; idx < ast.blocks.size(); idx++) {
        auto& block = ast.blocks[idx];

        m_table = block.symbolTable;
        for (auto& var : block.decls) {
            visit(*var);
            for (size_t next = idx + 1; next < ast.blocks.size(); next++) {
                ast.blocks[next].symbolTable->addReference(var->symbol);
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
        visit(*block.stmt);
    }
}

void SemanticAnalyzer::visit(AstForStmt& ast) {
    Sem::ForStmtPass(*this, ast);
}

void SemanticAnalyzer::visit(AstDoLoopStmt& ast) {
    RESTORE_ON_EXIT(m_table);
    ast.symbolTable = m_context.create<SymbolTable>(m_table);
    m_table = ast.symbolTable;

    for (auto& var : ast.decls) {
        visit(*var);
    }

    if (ast.expr) {
        expression(ast.expr);
        if (!ast.expr->type->isBoolean()) {
            fatalError("type '"_t
                + ast.expr->type->asString()
                + "' cannot be used as boolean");
        }
    }

    m_controlStack.push(ControlFlowStatement::Do);
    visit(*ast.stmt);
    m_controlStack.pop();
}

void SemanticAnalyzer::visit(AstControlFlowBranch& ast) {
    if (m_controlStack.find(ast.destination) == m_controlStack.cend()) {
        fatalError("control statement not found");
    }
}

//----------------------------------------
// Type (user defined)
//----------------------------------------

void SemanticAnalyzer::visit(AstTypeDecl& ast) {
    Sem::TypeDeclPass(*this, ast);
}

//----------------------------------------
// Attributes
//----------------------------------------

void SemanticAnalyzer::visit(AstAttributeList& /*ast*/) {
    llvm_unreachable("visitAttributeList");
}

void SemanticAnalyzer::visit(AstAttribute& /*ast*/) {
    llvm_unreachable("visitAttribute");
}

//----------------------------------------
// Types
//----------------------------------------

void SemanticAnalyzer::visit(AstTypeExpr& ast) {
    m_typePass.visit(ast);
}

//----------------------------------------
// Expressions
//----------------------------------------

void SemanticAnalyzer::expression(AstExpr*& ast, const TypeRoot* type) {
    visit(*ast);
    m_constantFolder.fold(ast);
    if (type != nullptr) {
        coerce(ast, type);
        m_constantFolder.fold(ast);
    }
}

void SemanticAnalyzer::visit(AstAssignExpr& ast) {
    visit(*ast.lhs);
    if (!ast.lhs->flags.assignable) {
        fatalError("Cannot assign");
    }
    expression(ast.rhs, ast.lhs->type);
}

void SemanticAnalyzer::visit(AstIdentExpr& ast) {
    auto* symbol = m_table->find(ast.name);
    if (symbol == nullptr) {
        fatalError("Unknown identifier "_t + ast.name);
    }

    const auto* type = symbol->type();
    if (type == nullptr) {
        fatalError("Identifier "_t + ast.name + " has unresolved type");
    }

    ast.type = type;
    ast.symbol = symbol;
    ast.flags = symbol->getFlags();
}

void SemanticAnalyzer::visit(AstCallExpr& ast) {
    visit(*ast.callable);

    const auto* type = dyn_cast<TypeFunction>(ast.callable->type);
    if (type == nullptr) {
        fatalError("Trying to call a non callable");
    }

    const auto& paramTypes = type->getParams();
    auto& args = ast.args;

    if (type->isVariadic()) {
        if (paramTypes.size() > args.size()) {
            fatalError("Argument count mismatch");
        }
    } else if (paramTypes.size() != args.size()) {
        fatalError("Argument count mismatch");
    }

    for (size_t index = 0; index < args.size(); index++) {
        if (index < paramTypes.size()) {
            expression(args[index], paramTypes[index]);
        } else {
            expression(args[index]);
        }
    }

    ast.type = type->getReturn();
}

void SemanticAnalyzer::visit(AstLiteralExpr& ast) {
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
    auto typeKind = std::visit(visitor, ast.value);
    ast.type = TypeRoot::fromTokenKind(typeKind);
}

//------------------------------------------------------------------
// Unary Expressions
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstUnaryExpr& ast) {
    expression(ast.expr);
    const auto* type = ast.expr->type;

    switch (ast.tokenKind) {
    case TokenKind::LogicalNot:
        if (type->isBoolean()) {
            ast.type = type;
            return;
        }
        fatalError("Applying unary NOT to non bool type");
    case TokenKind::Negate:
        if (type->isNumeric()) {
            ast.type = type;
            return;
        }
        fatalError("Applying unary negate to non-numeric type");
    default:
        llvm_unreachable("unknown unary operator");
    }
}

//------------------------------------------------------------------
// Dereference
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstDereference& ast) {
    // TODO dereference needs to return a reference to value, NOT value itself

    visit(*ast.expr);
    if (const auto* type = dyn_cast<TypePointer>(ast.expr->type)) {
        ast.type = type->getBase();
    } else {
        fatalError("dereferencing a non pointer");
    }

    ast.flags = ast.expr->flags;
    if (!ast.type->isPointer()) {
        ast.flags.dereferencable = false;
    }
}

//------------------------------------------------------------------
// AddressOf
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstAddressOf& ast) {
    visit(*ast.expr);
    if (!ast.expr->flags.addressable) {
        fatalError("Cannot take address");
    }
    ast.type = TypePointer::get(ast.expr->type);
    ast.flags = ast.expr->flags;
    ast.flags.dereferencable = true;
}

//------------------------------------------------------------------
// Member Access
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstMemberAccess& ast) {
    visit(*ast.lhs);
    const auto* type = ast.lhs->type;

    const TypeUDT* udt = nullptr;
    if (type->isUDT()) {
        udt = static_cast<const TypeUDT*>(type);
    } else if (const auto* ptr = dyn_cast<TypePointer>(type)) {
        if (ptr->getBase()->isUDT()) {
            udt = static_cast<const TypeUDT*>(ptr->getBase());
        }
    }

    if (udt == nullptr) {
        fatalError("Accessing member of non UDT type");
    }

    RESTORE_ON_EXIT(m_table);
    m_table = &udt->getSymbolTable();
    visit(*ast.rhs);
    ast.type = ast.rhs->type;
    ast.flags = ast.rhs->flags;
}

//------------------------------------------------------------------
// Binary Expressions
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstBinaryExpr& ast) {
    expression(ast.lhs);
    expression(ast.rhs);

    switch (Token::getOperatorType(ast.tokenKind)) {
    case OperatorType::Arithmetic:
        return arithmetic(ast);
    case OperatorType::Comparison:
        return comparison(ast);
    case OperatorType::Logical:
        return logical(ast);
    default:
        llvm_unreachable("invalid operator");
    }
}

void SemanticAnalyzer::arithmetic(AstBinaryExpr& ast) {
    const auto* left = ast.lhs->type;
    const auto* right = ast.rhs->type;

    if (!left->isNumeric() || !right->isNumeric()) {
        fatalError("Applying artithmetic operation to non numeric type");
    }

    const auto convert = [&](AstExpr*& expr, const TypeRoot* ty) {
        cast(expr, ty);
        m_constantFolder.fold(expr);
        ast.type = ty;
    };

    switch (left->compare(right)) {
    case TypeComparison::Incompatible:
        fatalError("Operator on incompatible types");
    case TypeComparison::Downcast:
        return convert(ast.rhs, left);
    case TypeComparison::Equal:
        ast.type = left;
        return;
    case TypeComparison::Upcast:
        return convert(ast.lhs, right);
    }
}

void SemanticAnalyzer::logical(AstBinaryExpr& ast) {
    const auto* left = ast.lhs->type;
    const auto* right = ast.rhs->type;

    if (!left->isBoolean() || !right->isBoolean()) {
        fatalError("Applying logical operator to non boolean type");
    }
    ast.type = left;
}

void SemanticAnalyzer::comparison(AstBinaryExpr& ast) {
    const auto* left = ast.lhs->type;
    const auto* right = ast.rhs->type;

    if (!canPerformBinary(ast.tokenKind, left, right)) {
        fatalError("Cannot apply operationg to types");
    }

    const auto convert = [&](AstExpr*& expr, const TypeRoot* ty) {
        cast(expr, ty);
        m_constantFolder.fold(expr);
        ast.type = TypeBoolean::get();
    };

    switch (left->compare(right)) {
    case TypeComparison::Incompatible:
        fatalError("Operator on incompatible types");
    case TypeComparison::Downcast:
        return convert(ast.rhs, left);
    case TypeComparison::Equal:
        ast.type = TypeBoolean::get();
        return;
    case TypeComparison::Upcast:
        return convert(ast.lhs, right);
    }
}

bool SemanticAnalyzer::canPerformBinary(TokenKind op, const TypeRoot* left, const TypeRoot* right) const noexcept {
    if (left->isBoolean() && right->isBoolean()) {
        return op == TokenKind::Equal || op == TokenKind::NotEqual;
    }

    if (left->isPointer() && right->isPointer()) {
        return op == TokenKind::Equal || op == TokenKind::NotEqual;
    }

    return left->isNumeric() && right->isNumeric();
}

//------------------------------------------------------------------
// Casting
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstCastExpr& ast) {
    visit(*ast.typeExpr);
    ast.type = ast.typeExpr->type;
    expression(ast.expr);

    if (ast.expr->type->compare(ast.type) == TypeComparison::Incompatible) {
        fatalError("Incompatible cast");
    }

    ast.flags = ast.expr->flags;
}

void SemanticAnalyzer::convert(AstExpr*& ast, const TypeRoot* type) {
    cast(ast, type);
    m_constantFolder.fold(ast);
}

void SemanticAnalyzer::coerce(AstExpr*& ast, const TypeRoot* type) {
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
    case TypeComparison::Equal:
        return;
    }
}

void SemanticAnalyzer::cast(AstExpr*& ast, const TypeRoot* type) {
    auto category = ast->flags;
    auto* cast = m_context.create<AstCastExpr>(
        ast->range,
        ast,
        nullptr,
        true);
    cast->type = type;
    cast->flags = category;
    ast = cast; // NOLINT
}

//------------------------------------------------------------------
// IfExpr
//------------------------------------------------------------------

void SemanticAnalyzer::visit(AstIfExpr& ast) {
    expression(ast.expr, TypeBoolean::get());
    expression(ast.trueExpr);
    expression(ast.falseExpr);

    const auto convert = [&](AstExpr*& expr, const TypeRoot* ty) {
        cast(expr, ty);
        m_constantFolder.fold(expr);
        ast.type = ty;
    };

    const auto* left = ast.trueExpr->type;
    const auto* right = ast.falseExpr->type;
    switch (left->compare(right)) {
    case TypeComparison::Incompatible:
        fatalError("Incompatible types");
    case TypeComparison::Downcast:
        return convert(ast.falseExpr, left);
    case TypeComparison::Equal:
        ast.type = left;
        return;
    case TypeComparison::Upcast:
        return convert(ast.trueExpr, right);
    }
}

//------------------------------------------------------------------
// Utils
//------------------------------------------------------------------

Symbol* SemanticAnalyzer::createNewSymbol(AstDecl& ast) {
    if (m_table->find(ast.name, false) != nullptr) {
        fatalError("Redefinition of "_t + ast.name);
    }
    auto* symbol = m_table->insert(ast.name);

    // alias?
    if (ast.attributes) {
        if (auto alias = ast.attributes->getStringLiteral("ALIAS")) {
            symbol->setAlias(*alias);
        }
    }

    return symbol;
}
