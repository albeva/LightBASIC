//
// Created by Albert Varaksin on 28/05/2021.
//
#pragma once
#include "pch.h"
#include "Ast/Ast.h"
#include "Gen/CodeGen.h"
#include "Gen/ValueHandler.h"

namespace lbc::Gen {

class ForStmtBuilder final {
public:
    NO_COPY_AND_MOVE(ForStmtBuilder);

    ForStmtBuilder(CodeGen& codeGen, AstForStmt* ast) noexcept;

    ~ForStmtBuilder() noexcept = default;

private:
    void declareVars() noexcept;
    void build() noexcept;
    void createBlocks() noexcept;
    ValueHandler getStep() noexcept;
    void checkDirection() noexcept;

    void makeCondition(bool incr) noexcept;
    void makeIteration(bool incr) noexcept;

    CodeGen& m_gen;
    llvm::IRBuilder<>& m_builder;
    llvm::LLVMContext& m_llvmContext;
    AstForStmt* m_ast;
    const AstForStmt::Direction m_direction;

    const TypeRoot* m_type = nullptr;
    llvm::Type* m_llvmType = nullptr;
    llvm::Value* m_isDecr = nullptr;

    llvm::BasicBlock* m_condBlock{};
    llvm::BasicBlock* m_bodyBlock{};
    llvm::BasicBlock* m_iterBlock{};
    llvm::BasicBlock* m_exitBlock{};

    ValueHandler m_iterator{};
    ValueHandler m_limit{};
    ValueHandler m_step{};
};

} // namespace lbc::Gen
