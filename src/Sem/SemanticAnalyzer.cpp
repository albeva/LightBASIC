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

[[noreturn]] static void error(const string& message) {
    std::cerr << message << std::endl;
    std::exit(EXIT_FAILURE);
}

SemanticAnalyzer::SemanticAnalyzer(llvm::LLVMContext& context)
  : m_context{ context } {}

void SemanticAnalyzer::visit(AstProgram* ast) {
    ast->symbolTable = make_unique<SymbolTable>(nullptr);

    m_table = ast->symbolTable.get();
    ast->stmtList->accept(this);
}

void SemanticAnalyzer::visit(AstStmtList* ast) {
    for (auto& stmt : ast->stmts) {
        stmt->accept(this);
    }
}

void SemanticAnalyzer::visit(AstAssignStmt* ast) {
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
}

void SemanticAnalyzer::visit(AstExprStmt* ast) {
    ast->expr->accept(this);
}

void SemanticAnalyzer::visit(AstVarDecl* ast) {
    auto* symbol = createIdentSymbol(ast->identExpr.get());

    // type expr?
    const TypeRoot* type = nullptr;
    if (ast->typeExpr) {
        ast->typeExpr->accept(this);
        type = ast->typeExpr->type;
    }

    // expression?
    if (ast->expr) {
        ast->expr->accept(this);

        if (type != nullptr) {
            if (type != ast->expr->type) {
                error("Type mismatch");
            }
        } else {
            type = ast->expr->type;
        }
    }

    // create function symbol
    symbol->setType(type);
    ast->identExpr->type = type;
    ast->type = type;
    ast->symbol = symbol;

    // alias?
    if (ast->attribs) {
        if (const auto* token = ast->attribs->getStringLiteral("ALIAS")) {
            symbol->setAlias(token->lexeme());
        }
    }
}

/**
 * Analyze function declaration
 */
void SemanticAnalyzer::visit(AstFuncDecl* ast) {
    auto* symbol = createIdentSymbol(ast->identExpr.get());

    // parameters
    std::vector<const TypeRoot*> paramTypes;
    paramTypes.reserve(ast->paramDecls.size());
    {
        RESTORE_ON_EXIT(m_table);
        ast->symbolTable = make_unique<SymbolTable>(nullptr);
        m_table = ast->symbolTable.get();
        for (auto& param : ast->paramDecls) {
            param->accept(this);
            paramTypes.emplace_back(param->type);
        }
    }

    // return typeExpr. subs don't have one so default to Void
    const TypeRoot* retType = nullptr;
    if (ast->retTypeExpr) {
        ast->retTypeExpr->accept(this);
        retType = ast->retTypeExpr->type;
    } else {
        retType = TypeVoid::get();
    }

    // create function symbol
    const auto* type = TypeFunction::get(retType, std::move(paramTypes));
    symbol->setType(type);
    ast->identExpr->type = type;
    ast->type = type;
    ast->symbol = symbol;

    // alias?
    if (ast->attribs) {
        if (const auto* token = ast->attribs->getStringLiteral("ALIAS")) {
            symbol->setAlias(token->lexeme());
        }
    }
}

void SemanticAnalyzer::visit(AstFuncParamDecl* ast) {
    auto* symbol = createIdentSymbol(ast->identExpr.get());

    ast->typeExpr->accept(this);
    symbol->setType(ast->typeExpr->type);

    ast->symbol = symbol;
    ast->type = symbol->type();
    ast->identExpr->type = symbol->type();
}

void SemanticAnalyzer::visit(AstAttributeList* ast) {
    for (auto& attrib: ast->attribs) {
        attrib->accept(this);
    }
}

void SemanticAnalyzer::visit(AstAttribute* ast) {
    for (auto& arg: ast->argExprs) {
        arg->accept(this);
    }
}

void SemanticAnalyzer::visit(AstTypeExpr* ast) {
    ast->type = TypeRoot::fromTokenKind(ast->token->kind());
}

void SemanticAnalyzer::visit(AstIdentExpr* ast) {
    const auto& name = ast->token->lexeme();
    auto* symbol = m_table->find(name, true);
    if (symbol == nullptr) {
        error("Unknown identifier "s + string(name));
    }

    ast->symbol = symbol;
    ast->type = symbol->type();
}

void SemanticAnalyzer::visit(AstCallExpr* ast) {
    ast->identExpr->accept(this);
    auto* symbol = ast->identExpr->symbol;

    const auto* type = llvm::dyn_cast<TypeFunction>(symbol->type());
    if (type == nullptr) {
        error("Identifier "s + string(symbol->name()) + " is not a callable type"s);
    }

    const auto& params = type->paramTypes();
    if (params.size() != ast->argExprs.size()) {
        error("Argument count mismatch");
    }

    size_t index = 0;
    for (auto& arg: ast->argExprs) {
        arg->accept(this);
        if (params[index] != arg->type) {
            error("Type mismatch");
        }
        index++;
    }

    ast->type = type->retType();
}

void SemanticAnalyzer::visit(AstLiteralExpr* ast) {
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
    default:
        error("Unsupported literal typeExpr");
    }
}

Symbol* SemanticAnalyzer::createIdentSymbol(AstIdentExpr* identExpr) {
    // visit(identExpr);
    const auto& name = identExpr->token->lexeme();

    auto* symbol = m_table->find(name, false);
    if (symbol != nullptr) {
        error("Redefinition of " + string(name));
    }
    symbol = m_table->insert(make_unique<Symbol>(name));

    identExpr->symbol = symbol;
    return symbol;
}
