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

class SemanticAnalyzer final: public AstVisitor {
public:
    SemanticAnalyzer();
    ~SemanticAnalyzer();

    #define IMPL_VISITOR(NODE, ...) virtual void visit(Ast##NODE* ast);
    AST_CONTENT_NODES(IMPL_VISITOR)
    #undef IMPL_VISITOR

private:
    string_view m_identifier;
    SymbolTable* m_table;
    const TypeRoot* m_type;
    Symbol* m_symbol;
};

} // namespace lbc