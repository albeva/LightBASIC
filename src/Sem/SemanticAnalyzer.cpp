//
// Created by Albert Varaksin on 08/07/2020.
//
#include "SemanticAnalyzer.h"
#include "Ast/Ast.h"
#include "Lexer/Token.h"
#include "Symbol/Symbol.h"
#include "Symbol/SymbolTable.h"
#include "Type/Type.h"
using namespace lbc;

SemanticAnalyzer::SemanticAnalyzer(Context& context)
: m_context{ context } {}

void SemanticAnalyzer::visit(AstModule* ast) {
    m_astRootModule = ast;
    m_fileId = ast->fileId;
    ast->symbolTable = make_unique<SymbolTable>(nullptr);

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
    visitExpr(ast->expr.get());

    if (ast->identExpr->type != ast->expr->type) {
        fatalError("Tye mismatch in assignment");
    }
}

void SemanticAnalyzer::visitExprStmt(AstExprStmt* ast) {
    visitExpr(ast->expr.get());
}

void SemanticAnalyzer::visitVarDecl(AstVarDecl* ast) {
    auto* symbol = createNewSymbol(ast->token.get());

    // m_type expr?
    const TypeRoot* type = nullptr;
    if (ast->typeExpr) {
        visitTypeExpr(ast->typeExpr.get());
        type = ast->typeExpr->type;
    }

    // expression?
    if (ast->expr) {
        visitExpr(ast->expr.get());

        if (type != nullptr) {
            if (type != ast->expr->type) {
                fatalError("TypeFirst mismatch");
            }
        } else {
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

/**
 * Analyze function declaration
 */
void SemanticAnalyzer::visitFuncDecl(AstFuncDecl* ast) {
    auto* symbol = createNewSymbol(ast->token.get());

    // parameters
    std::vector<const TypeRoot*> paramTypes;
    paramTypes.reserve(ast->paramDecls.size());
    {
        RESTORE_ON_EXIT(m_table)
        ast->symbolTable = make_unique<SymbolTable>(m_table);
        m_table = ast->symbolTable.get();
        for (auto& param : ast->paramDecls) {
            visitFuncParamDecl(param.get());
            paramTypes.emplace_back(param->symbol->type());
        }
    }

    // return typeExpr. subs don't have one so default to Void
    const TypeRoot* retType = nullptr;
    if (ast->retTypeExpr) {
        visitTypeExpr(ast->retTypeExpr.get());
        retType = ast->retTypeExpr->type;
    } else {
        retType = TypeVoid::get();
    }

    // create function symbol
    const auto* type = TypeFunction::get(retType, std::move(paramTypes), ast->variadic);
    symbol->setType(type);
    ast->symbol = symbol;

    // alias?
    if (ast->attributes) {
        if (const auto* token = ast->attributes->getStringLiteral("ALIAS")) {
            symbol->setAlias(token->lexeme());
        }
    }
}

void SemanticAnalyzer::visitFuncParamDecl(AstFuncParamDecl* ast) {
    auto* symbol = createNewSymbol(ast->token.get());

    visitTypeExpr(ast->typeExpr.get());
    symbol->setType(ast->typeExpr->type);

    ast->symbol = symbol;
}

void SemanticAnalyzer::visitFuncStmt(AstFuncStmt* ast) {
    visitFuncDecl(ast->decl.get());

    auto sym = ast->decl->symbol;
    if (!m_astRootModule->hasImplicitMain && sym->name() == "MAIN" && sym->alias().empty()) {
        sym->setAlias("main");
    }

    RESTORE_ON_EXIT(m_table);
    m_table = ast->decl->symbolTable.get();

    visitStmtList(ast->stmtList.get());
}

void SemanticAnalyzer::visitAttributeList(AstAttributeList* ast) {
    for (auto& attrib : ast->attribs) {
        visitAttribute(attrib.get());
    }
}

void SemanticAnalyzer::visitAttribute(AstAttribute* ast) {
    for (auto& arg : ast->argExprs) {
        visitLiteralExpr(arg.get());
    }
}

void SemanticAnalyzer::visitTypeExpr(AstTypeExpr* ast) {
    ast->type = TypeRoot::fromTokenKind(ast->token->kind());
}

void SemanticAnalyzer::visitIdentExpr(AstIdentExpr* ast) {
    const auto& name = ast->token->lexeme();
    auto* symbol = m_table->find(name, true);

    if (symbol == nullptr) {
        fatalError("Unknown identifier "s + string(name));
    }

    if (symbol->type() == nullptr) {
        fatalError("Identifier "s + string(name) + " has unresolved m_type");
    }

    ast->symbol = symbol;
    ast->type = symbol->type();
}

void SemanticAnalyzer::visitCallExpr(AstCallExpr* ast) {
    visitIdentExpr(ast->identExpr.get());

    auto* symbol = ast->identExpr->symbol;

    const auto* type = dyn_cast<TypeFunction>(symbol->type());
    if (type == nullptr) {
        fatalError("Identifier "s + string(symbol->name()) + " is not a callable m_type"s);
    }

    const auto& paramTypes = type->paramTypes();
    auto& args = ast->argExprs;

    if (type->variadic()) {
        if (paramTypes.size() > args.size()) {
            fatalError("Argument count mismatch");
        }
    } else if (paramTypes.size() != args.size()) {
        fatalError("Argument count mismatch");
    }

    for (size_t index = 0; index < args.size(); index++) {
        visitExpr(args[index].get());
        if (index < paramTypes.size()) {
            if (paramTypes[index] != args[index]->type) {
                fatalError("TypeFirst mismatch");
            }
        }
    }

    ast->type = type->retType();
}

void SemanticAnalyzer::visitLiteralExpr(AstLiteralExpr* ast) {
    switch (ast->token->kind()) {
    case TokenKind::StringLiteral:
        ast->type = TypeZString::get();
        break;
    case TokenKind::BooleanLiteral:
        ast->type = TypeBool::get();
        break;
    case TokenKind::NullLiteral:
        ast->type = TypePointer::get(TypeAny::get());
        break;
    case TokenKind::NumberLiteral:
        ast->type = TypeRoot::fromTokenKind(TokenKind::Integer);
        break;
    default:
        fatalError("Unsupported literal m_type");
    }
}

Symbol* SemanticAnalyzer::createNewSymbol(Token* token) {
    if (m_table->find(token->lexeme(), false) != nullptr) {
        fatalError("Redefinition of " + string(token->lexeme()));
    }
    return m_table->insert(token->lexeme());
}
