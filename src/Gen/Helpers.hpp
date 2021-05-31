//
// Created by Albert Varaksin on 28/05/2021.
//
#pragma once


namespace lbc {
class TypeRoot;
enum class TokenKind;

namespace Gen {
    [[nodiscard]] llvm::CmpInst::Predicate getCmpPred(const TypeRoot* type, TokenKind op) noexcept;
    [[nodiscard]] llvm::Instruction::BinaryOps getBinOpPred(const TypeRoot* type, TokenKind op) noexcept;
} // namespace Gen
} // namespace lbc
