//
// Created by Albert on 28/05/2021.
//
#pragma once
#include "Ast/Ast.hpp"
#include "Gen/CodeGen.hpp"
#include "pch.hpp"

namespace lbc::Gen {

class BinaryExprBuilder final {
public:
    NO_COPY_AND_MOVE(BinaryExprBuilder);

    BinaryExprBuilder(CodeGen& gen, AstBinaryExpr* ast) noexcept;
    ~BinaryExprBuilder() noexcept = default;

    llvm::Value* build() noexcept;

private:
    llvm::Value* comparison() noexcept;
    llvm::Value* arithmetic() noexcept;
    llvm::Value* logical() noexcept;

    CodeGen& m_gen;
    llvm::IRBuilder<>& m_builder;
    llvm::LLVMContext& m_llvmContext;
    AstBinaryExpr* m_ast;
};

} // namespace lbc::Gen
