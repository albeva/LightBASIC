//
// Created by Albert on 08/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"

namespace lbc {

class Token;
class Symbol;
class SymbolTable;
class TypeRoot;

class SemanticAnalyzer final : public AstVisitor {
public:
    explicit SemanticAnalyzer(llvm::LLVMContext& context);

#define IMPL_VISITOR(NODE, ...) virtual void visit(Ast##NODE* ast);
    AST_CONTENT_NODES(IMPL_VISITOR)
#undef IMPL_VISITOR

private:
    llvm::LLVMContext& m_context;
    string_view m_identifier;
    SymbolTable* m_table = nullptr;
    const TypeRoot* m_type = nullptr;
    Symbol* m_symbol = nullptr;
};

} // namespace lbc