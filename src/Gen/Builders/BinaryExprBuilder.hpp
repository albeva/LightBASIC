//
// Created by Albert on 28/05/2021.
//
#pragma once
#include "pch.hpp"
#include "Ast/Ast.hpp"
#include "Builder.hpp"
#include "Gen/CodeGen.hpp"

namespace lbc::Gen {

class BinaryExprBuilder final : Builder<AstBinaryExpr> {
public:
    using Builder::Builder;
    llvm::Value* build() noexcept;

private:
    llvm::Value* comparison() noexcept;
    llvm::Value* arithmetic() noexcept;
    llvm::Value* logical() noexcept;
};

} // namespace lbc::Gen
