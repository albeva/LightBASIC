//
// Created by Albert Varaksin on 08/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"

namespace lbc {

class Token;
class Symbol;
class SymbolTable;
class TypeRoot;
class Context;

class SemanticAnalyzer final : private NonCopyable, public AstVisitor<SemanticAnalyzer> {
public:
    SemanticAnalyzer(Context& context, unsigned fileId);

    AST_DECLARE_ALL_ROOT_VISIT_METHODS()
private:
    [[nodiscard]] Symbol* createNewSymbol(Token* identExpr, SymbolTable* table = nullptr);

    Context& m_context;
    unsigned m_fileId;
    SymbolTable* m_table = nullptr;
    SymbolTable* m_rootTable = nullptr;
};

} // namespace lbc
