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
  m_ast(ast),
  m_symbol{sem.createNewSymbol(ast)} {
    auto* current = m_sem.getSymbolTable();

    ast.symbolTable = make_unique<SymbolTable>(nullptr);
    m_sem.setSymbolTable(ast.symbolTable.get());
    declareMembers();
    TypeUDT::get(*m_symbol, *ast.symbolTable);

    m_sem.setSymbolTable(current);
}

void TypeDeclPass::declareMembers() {
    int idx = 0;
    for (const auto& decl : m_ast.decls) {
        m_sem.visit(*decl);
        decl->symbol->setIndex(idx++);
        decl->symbol->setParent(m_symbol);
    }
}
