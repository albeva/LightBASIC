//
// Created by Albert on 29/05/2021.
//
#include "TypeDeclPass.hpp"
#include "Ast/Ast.hpp"
#include "Sem/SemanticAnalyzer.hpp"
#include "Type/Type.hpp"
#include "Type/TypeUdt.hpp"
using namespace lbc;
using namespace Sem;

TypeDeclPass::TypeDeclPass(SemanticAnalyzer& sem, AstTypeDecl& ast)
: m_sem(sem),
  m_ast(ast) {
    auto* symbol = m_sem.createNewSymbol(ast);

    auto* current = m_sem.getSymbolTable();

    ast.symbolTable = make_unique<SymbolTable>(nullptr);
    m_sem.setSymbolTable(ast.symbolTable.get());
    declareMembers();
    TypeUDT::get(*symbol, *ast.symbolTable);

    m_sem.setSymbolTable(current);
}

void TypeDeclPass::declareMembers() {
    for (const auto& decl : m_ast.decls) {
        m_sem.visit(*decl);
    }
}
