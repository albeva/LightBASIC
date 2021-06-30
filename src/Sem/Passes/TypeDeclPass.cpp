//
// Created by Albert on 29/05/2021.
//
#include "TypeDeclPass.hpp"
#include "Ast/Ast.hpp"
#include "Driver/Context.hpp"
#include "Sem/SemanticAnalyzer.hpp"
#include "Type/Type.hpp"
#include "Type/TypeUdt.hpp"
using namespace lbc;
using namespace Sem;

TypeDeclPass::TypeDeclPass(SemanticAnalyzer& sem, AstTypeDecl& ast)
: m_sem(sem),
  m_ast(ast),
  m_symbol{ sem.createNewSymbol(ast) } {
    auto* current = m_sem.getSymbolTable();

    bool packed = false;
    if (ast.attributes != nullptr) {
        packed = ast.attributes->exists("PACKED");
    }

    ast.symbolTable = m_sem.getContext().create<SymbolTable>(current);
    m_sem.setSymbolTable(ast.symbolTable);
    declareMembers();
    ast.symbolTable->setParent(nullptr);
    TypeUDT::get(m_sem.getContext(), *m_symbol, *ast.symbolTable, packed);

    m_sem.setSymbolTable(current);
}

void TypeDeclPass::declareMembers() {
    for (const auto& decl : m_ast.decls) {
        m_sem.visit(*decl);
        decl->symbol->setParent(m_symbol);
    }
}
