//
// Created by Albert Varaksin on 28/05/2021.
//
#pragma once
#include "Ast/Ast.hpp"
#include "Gen/CodeGen.hpp"
#include "pch.hpp"

namespace lbc::Gen {

class DoLoopBuilder final {
public:
    NO_COPY_AND_MOVE(DoLoopBuilder);

    DoLoopBuilder(CodeGen& gen, AstDoLoopStmt* ast) noexcept;
    ~DoLoopBuilder() noexcept = default;

private:
    void makeCondition(bool isUntil) noexcept;
    void build() noexcept;

    CodeGen& m_gen;
    llvm::IRBuilder<>& m_builder;
    llvm::LLVMContext& m_llvmContext;
    AstDoLoopStmt* m_ast;

    llvm::BasicBlock* m_bodyBlock;
    llvm::BasicBlock* m_condBlock;
    llvm::BasicBlock* m_exitBlock;
    llvm::BasicBlock* m_continueBlock;
};

} // namespace lbc::Gen
