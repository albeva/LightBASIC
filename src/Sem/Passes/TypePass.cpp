//
// Created by Albert Varaksin on 22/05/2021.
//
#include "TypePass.hpp"
#include "Ast/Ast.hpp"
#include "Sem/SemanticAnalyzer.hpp"
#include "Symbol/Symbol.hpp"
#include "Symbol/SymbolTable.hpp"
#include "Type/Type.hpp"
#include "Type/TypeUdt.hpp"

using namespace lbc;
using namespace Sem;

void TypePass::visit(AstTypeExpr& ast) {
    const TypeRoot* type = nullptr;
    if (ast.tokenKind == TokenKind::Identifier) {
        auto* table = m_sem.getSymbolTable();
        // TODO: Support nested names
        auto* sym = table->find(ast.ident->name);
        if (sym == nullptr) {
            fatalError("Undefined type "_t + ast.ident->name);
        }
        if (const auto* udt = dyn_cast<TypeUDT>(sym->type())) {
            type = udt;
        } else {
            fatalError(""_t + sym->name() + " is not a type");
        }
    } else {
        type = TypeRoot::fromTokenKind(ast.tokenKind);
    }
    for (auto deref = 0; deref < ast.dereference; deref++) {
        type = TypePointer::get(m_sem.getContext(), type);
    }
    ast.type = type;
}
