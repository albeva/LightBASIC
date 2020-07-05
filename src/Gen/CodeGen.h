//
// Created by Albert on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"


namespace lbc {

class CodeGen final: public AstVisitor {
public:
    CodeGen();
    ~CodeGen();

    #define IMPL_VISITOR(NODE, ...) virtual void visit(const Ast##NODE* ast);
    AST_CONTENT_NODES(IMPL_VISITOR)
    #undef IMPL_VISITOR

private:
    llvm::LLVMContext m_context;
    llvm::IRBuilder<> m_builder;
    unique_ptr<llvm::Module> m_module;
    llvm::Value* m_value;
    std::unordered_map<string, llvm::Value *> m_stringLiterals;
};

} // namespace lbc
