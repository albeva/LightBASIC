//
// Created by Albert Varaksin on 28/05/2021.
//
#pragma once
#include "pch.hpp"
#include "Symbol/Symbol.hpp"
#include <llvm/ADT/PointerUnion.h>

namespace lbc {
class CodeGen;
class Symbol;
struct AstExpr;

namespace Gen {
    namespace value_handler_detail {
        using ValuePtr = llvm::PointerIntPair<llvm::Value*, 1, bool>;
    } // namespace value_handler_detail

    class ValueHandler final : llvm::PointerUnion<value_handler_detail::ValuePtr, Symbol*> {
    public:
        /// Create temporary allocated variable - it is not inserted into symbol table
        static ValueHandler createTemp(CodeGen& gen, AstExpr& expr, StringRef name = "") noexcept;

        /// Create temporary variable if expression is not a constant
        static ValueHandler createTempOrConstant(CodeGen& gen, AstExpr& expr, StringRef name = "") noexcept;

        constexpr ValueHandler() noexcept = default;
        ValueHandler(CodeGen* gen, Symbol* symbol) noexcept;
        ValueHandler(CodeGen* gen, llvm::Value* value) noexcept;

        [[nodiscard]] llvm::Value* get() noexcept;
        void set(llvm::Value* val) noexcept;

        [[nodiscard]] constexpr inline bool isValid() const noexcept {
            return m_gen != nullptr && !isNull();
        }

    private:
        ValueHandler(CodeGen* gen, value_handler_detail::ValuePtr ptr) noexcept;

        CodeGen* m_gen = nullptr;
    };

} // namespace Gen
} // namespace lbc
