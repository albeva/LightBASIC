//
// Created by Albert on 28/05/2021.
//
#pragma once
#include "Ast/Ast.hpp"
#include "Gen/CodeGen.hpp"
#include "pch.hpp"

namespace lbc::Gen {

class IfStmtBuilder final {
public:
    NO_COPY_AND_MOVE(IfStmtBuilder);

    IfStmtBuilder(CodeGen& gen, AstIfStmt* ast) noexcept;
    ~IfStmtBuilder() noexcept = default;

private:
    void build() noexcept;

    CodeGen& m_gen;
    llvm::IRBuilder<>& m_builder;
    llvm::LLVMContext& m_llvmContext;
    AstIfStmt* m_ast;
};

} // namespace lbc::Gen
