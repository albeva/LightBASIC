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
    explicit SemanticAnalyzer(llvm::LLVMContext& context, llvm::SourceMgr& srcMgr, unsigned fileId);

#define IMPL_VISITOR(NODE, ...) virtual void visit(Ast##NODE* ast);
    AST_CONTENT_NODES(IMPL_VISITOR)
#undef IMPL_VISITOR

private:
    [[nodiscard]] Symbol* createNewSymbol(Token* identExpr, SymbolTable* table = nullptr);

    llvm::LLVMContext& m_context;
    llvm::SourceMgr& m_srcMgr;
    unsigned m_fileId;
    SymbolTable* m_table = nullptr;
    SymbolTable* m_rootTable = nullptr;
};

} // namespace lbc
