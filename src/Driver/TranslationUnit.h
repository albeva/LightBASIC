//
// Created by Albert Varaksin on 08/05/2021.
//
#pragma once
#include "pch.h"
#include "Ast/Ast.h"
#include "Source.h"

namespace lbc {

struct TranslationUnit final {
    NO_COPY_AND_MOVE(TranslationUnit)

    TranslationUnit(unique_ptr<llvm::Module>&& m, const Source* src, unique_ptr<AstModule>&& tree) noexcept
    : llvmModule{ std::move(m) }, source{ src }, ast{ std::move(tree) } {}

    ~TranslationUnit() = default;

    unique_ptr<llvm::Module> llvmModule;
    const Source* source;
    unique_ptr<AstModule> ast;
};

} // namespace lbc
