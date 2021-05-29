//
// Created by Albert on 29/05/2021.
//
#pragma once
#include "pch.hpp"
#include "Ast/Ast.hpp"
#include "Gen/CodeGen.hpp"

namespace lbc::Gen {

template<typename T, std::enable_if_t<std::is_base_of_v<AstRoot, T>, int> = 0>
class Builder {
public:
    Builder(CodeGen& gen, T* ast) noexcept
    : m_gen{ gen },
      m_builder{ gen.getBuilder() },
      m_llvmContext{ m_builder.getContext() },
      m_ast{ ast } {}

    CodeGen& m_gen;
    llvm::IRBuilder<>& m_builder;
    llvm::LLVMContext& m_llvmContext;
    T* m_ast;
};

} // namespace lbc::Gen
