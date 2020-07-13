//
// Created by Albert on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"

namespace lbc {

class CodeGen final : public AstVisitor {
public:
    [[nodiscard]] llvm::Module* module() const { return m_module.get(); }

    explicit CodeGen(llvm::LLVMContext& context, llvm::SourceMgr& srcMgr, unsigned fileId);

#define IMPL_VISITOR(NODE, ...) virtual void visit(Ast##NODE* ast);
    AST_CONTENT_NODES(IMPL_VISITOR)
#undef IMPL_VISITOR

private:
    using ValueMap = std::unordered_map<string, llvm::Value*>;
    using LiteralMap = std::unordered_map<string_view, llvm::Constant*>;

    static llvm::Value* getStoreValue(AstIdentExpr* identExpr);

    llvm::LLVMContext& m_context;
    llvm::SourceMgr& m_srcMgr;
    unsigned m_fileId;

    llvm::IRBuilder<> m_builder;
    unique_ptr<llvm::Module> m_module;
    llvm::Value* m_value = nullptr;
    llvm::Function* m_function = nullptr;
    llvm::Function* m_startupFn = nullptr;
    llvm::BasicBlock* m_startupBlock = nullptr;

    llvm::BasicBlock* m_block = nullptr;
    ValueMap m_values;
    LiteralMap m_stringLiterals;
};

} // namespace lbc
