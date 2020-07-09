//
// Created by Albert on 08/07/2020.
//
#include "SemanticAnalyzer.h"
#include "Lexer/Token.h"
#include "Ast/Ast.h"
#include "Type/Type.h"
#include "Symbol/Symbol.h"
#include "Symbol/SymbolTable.h"
using namespace lbc;

[[noreturn]]
static void error(const string& message) {
    std::cerr << message << std::endl;
    std::exit(EXIT_FAILURE);
}

SemanticAnalyzer::SemanticAnalyzer(llvm::LLVMContext& context)
: m_context{context} {}

void SemanticAnalyzer::visit(AstProgram *ast) {
    ast->symbolTable = make_unique<SymbolTable>(nullptr);

    m_table = ast->symbolTable.get();
    ast->body->accept(this);
}

void SemanticAnalyzer::visit(AstStmtList *ast) {
    for (const auto& stmt: ast->stmts) {
        stmt->accept(this);
    }
}

void SemanticAnalyzer::visit(AstAssignStmt *ast) {
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
}

void SemanticAnalyzer::visit(AstExprStmt *ast) {
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
}

void SemanticAnalyzer::visit(AstVarDecl *ast) {
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
}

/**
 * Analyze function declaration
 */
void SemanticAnalyzer::visit(AstFuncDecl *ast) {
    // name
    ast->ident->accept(this);
    auto name = m_identifier;

    // already declared?
    if (m_table->exists(name)) {
        error("Redefinition of "s + string(name));
    }

    // parameters
    std::vector<const TypeRoot*> paramTypes;
    paramTypes.reserve(ast->params.size());
    {
        RESTORE_ON_EXIT(m_table);
        ast->symbolTable = make_unique<SymbolTable>(nullptr);
        m_table = ast->symbolTable.get();
        for (auto& param: ast->params) {
            param->accept(this);
            paramTypes.emplace_back(m_type);
        }
    }

    // return type. subs don't have one so default to Void
    const TypeRoot* retType = nullptr;
    if (ast->type) {
        ast->type->accept(this);
        retType = m_type;
    } else {
        retType = TypeVoid::get();
    }

    // create function symbol
    m_type = TypeFunction::get(retType, std::move(paramTypes));
    m_symbol = m_table->insert(make_unique<Symbol>(name, m_type));
}

void SemanticAnalyzer::visit(AstFuncParamDecl *ast) {
    ast->ident->accept(this);
    auto name = m_identifier;

    if (m_table->exists(name)) {
        error("duplicate parameter "s + string(name));
    }

    ast->type->accept(this);
    m_symbol = m_table->insert(make_unique<Symbol>(name, m_type));
}

void SemanticAnalyzer::visit(AstAttributeList *ast) {
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
}

void SemanticAnalyzer::visit(AstAttribute *ast) {
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
}

void SemanticAnalyzer::visit(AstTypeExpr *ast) {
    switch (ast->token->kind()) {
        #define CASE_PRIMITIVE(id, str, kind, ...) case TokenKind::id: \
            m_type = Type##kind::get(); \
            return;
        PRIMITIVE_TYPES(CASE_PRIMITIVE)
        #undef TO_PRIMITIVE_TYPE

        #define CASE_INTEGER(id, str, kind, bits, isSigned, ...) case TokenKind::id: \
            m_type = Type##kind::get(bits, isSigned); \
            return;
        INTEGER_TYPES(CASE_INTEGER)
        #undef CASE_INTEGER

        #define CASE_FLOATINGPOINT(id, str, kind, bits, ...) case TokenKind::id: \
            m_type = Type##kind::get(bits); \
            return;
        FLOATINGPOINT_TYPES(CASE_FLOATINGPOINT)
        #undef CASE_INTEGER
    default:
        error("Unknown type "s + string(ast->token->lexeme()));
    }
}

void SemanticAnalyzer::visit(AstIdentExpr *ast) {
    m_identifier = ast->token->lexeme();
}

void SemanticAnalyzer::visit(AstCallExpr *ast) {
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
}

void SemanticAnalyzer::visit(AstLiteralExpr *ast) {
    std::cout << "Not implemented " << __PRETTY_FUNCTION__ << '\n';
}

