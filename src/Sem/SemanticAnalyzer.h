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

class SemanticAnalyzer final : public AstVisitor<SemanticAnalyzer> {
public:
    explicit SemanticAnalyzer(Context& context);

    AST_DECLARE_ALL_ROOT_VISIT_METHODS()
private:
    [[nodiscard]] Symbol* createNewSymbol(AstDecl* ast, Token* identExpr);

    Context& m_context;
    unsigned int m_fileId = ~0U;
    AstModule* m_astRootModule = nullptr;
    AstFuncDecl* m_function = nullptr;
    SymbolTable* m_table = nullptr;
    SymbolTable* m_rootTable = nullptr;
};

} // namespace lbc
