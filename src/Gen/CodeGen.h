//
// Created by Albert on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"


namespace lbc {

class CodeGen final: public AstVisitor {
public:
    explicit CodeGen(llvm::LLVMContext& context);

    #define IMPL_VISITOR(NODE, ...) virtual void visit(Ast##NODE* ast);
    AST_CONTENT_NODES(IMPL_VISITOR)
    #undef IMPL_VISITOR

private:
    llvm::Function* getOrCreate(AstCallExpr* ast);

    llvm::LLVMContext& m_context;
    llvm::IRBuilder<> m_builder;
    unique_ptr<llvm::Module> m_module;
    llvm::Value* m_value = nullptr;
    llvm::Function* m_function = nullptr;
    llvm::BasicBlock* m_block = nullptr;
    std::unordered_map<string, llvm::Value *> m_values;
};

} // namespace lbc
