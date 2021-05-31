//
// Created by Albert Varaksin on 08/05/2021.
//
#pragma once
#include "pch.hpp"
#include "Ast/Ast.hpp"
#include "Source.hpp"

namespace lbc {

struct TranslationUnit final {
    NO_COPY_AND_MOVE(TranslationUnit)

    TranslationUnit(unique_ptr<llvm::Module> module, const Source* src, unique_ptr<AstModule> tree) noexcept
    : llvmModule{ std::move(module) }, source{ src }, ast{ std::move(tree) } {}

    ~TranslationUnit() noexcept = default;

    unique_ptr<llvm::Module> llvmModule;
    const Source* source;
    unique_ptr<AstModule> ast;
};

} // namespace lbc
