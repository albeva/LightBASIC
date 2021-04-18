//
// Created by Albert on 08/07/2020.
//
#include "SemanticAnalyzer.h"
#include "Ast/Ast.h"
#include "Lexer/Token.h"
#include "Symbol/Symbol.h"
#include "Symbol/SymbolTable.h"
#include "Type/Type.h"
using namespace lbc;

SemanticAnalyzer::SemanticAnalyzer(Context& context, unsigned fileId)
: m_context{ context },
  m_fileId{ fileId } {}

void SemanticAnalyzer::visitProgram(AstProgram* ast) {
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

    // type expr?
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
    auto* symbol = createNewSymbol(ast->token.get(), m_rootTable);

    // parameters
    std::vector<const TypeRoot*> paramTypes;
    paramTypes.reserve(ast->paramDecls.size());
    {
        RESTORE_ON_EXIT(m_table)
        ast->symbolTable = make_unique<SymbolTable>(nullptr);
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
        fatalError("Identifier "s + string(name) + " has unresolved type");
    }

    ast->symbol = symbol;
    ast->type = symbol->type();
}

void SemanticAnalyzer::visitCallExpr(AstCallExpr* ast) {
    visitIdentExpr(ast->identExpr.get());

    auto* symbol = ast->identExpr->symbol;

    const auto* type = dyn_cast<TypeFunction>(symbol->type());
    if (type == nullptr) {
        fatalError("Identifier "s + string(symbol->name()) + " is not a callable type"s);
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
        fatalError("Unsupported literal type");
    }
}

Symbol* SemanticAnalyzer::createNewSymbol(Token* token, SymbolTable* table) {
    if (table == nullptr) {
        table = m_table;
    }

    auto* symbol = table->find(token->lexeme(), false);
    if (symbol != nullptr) {
        fatalError("Redefinition of " + string(token->lexeme()));
    }

    symbol = table->insert(make_unique<Symbol>(token->lexeme()));
    return symbol;
}
