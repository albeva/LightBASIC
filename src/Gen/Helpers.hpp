//
// Created by Albert Varaksin on 28/05/2021.
//
#pragma once
#include "pch.hpp"

namespace lbc {
class TypeRoot;
enum class TokenKind;

namespace Gen {
    [[nodiscard]] llvm::CmpInst::Predicate getCmpPred(const TypeRoot* type, TokenKind op);
    [[nodiscard]] llvm::Instruction::BinaryOps getBinOpPred(const TypeRoot* type, TokenKind op);
} // namespace Gen
} // namespace lbc
