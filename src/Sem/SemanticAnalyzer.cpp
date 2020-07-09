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
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
}

void SemanticAnalyzer::visit(AstVarDecl* ast) {
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
}

/**
 * Analyze function declaration
 */
void SemanticAnalyzer::visit(AstFuncDecl* ast) {
    // name
    ast->identExpr->accept(this);
    auto name = m_identifier;

    // already declared?
    if (m_table->exists(name)) {
        error("Redefinition of "s + string(name));
    }

    // parameters
    std::vector<const TypeRoot*> paramTypes;
    paramTypes.reserve(ast->paramDecls.size());
    {
        RESTORE_ON_EXIT(m_table);
        ast->symbolTable = make_unique<SymbolTable>(nullptr);
        m_table = ast->symbolTable.get();
        for (auto& param : ast->paramDecls) {
            param->accept(this);
            paramTypes.emplace_back(m_type);
        }
    }

    // return typeExpr. subs don't have one so default to Void
    const TypeRoot* retType = nullptr;
    if (ast->retTypeExpr) {
        ast->retTypeExpr->accept(this);
        retType = m_type;
    } else {
        retType = TypeVoid::get();
    }

    // create function symbol
    m_type = TypeFunction::get(retType, std::move(paramTypes));
    m_symbol = m_table->insert(make_unique<Symbol>(name, m_type));
    ast->symbol = m_symbol;

    // alias?
    if (ast->attribs) {
        if (const auto* token = ast->attribs->getStringLiteral("ALIAS")) {
            m_symbol->setAlias(token->lexeme());
        }
    }
}

void SemanticAnalyzer::visit(AstFuncParamDecl* ast) {
    ast->identExpr->accept(this);
    auto name = m_identifier;

    if (m_table->exists(name)) {
        error("duplicate parameter "s + string(name));
    }

    ast->typeExpr->accept(this);
    m_symbol = m_table->insert(make_unique<Symbol>(name, m_type));
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
    m_type = TypeRoot::fromTokenKind(ast->token->kind());
    ast->type = m_type;
}

void SemanticAnalyzer::visit(AstIdentExpr* ast) {
    m_identifier = ast->token->lexeme();
}

void SemanticAnalyzer::visit(AstCallExpr* ast) {
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
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
