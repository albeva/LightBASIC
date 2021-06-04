//
// Created by Albert Varaksin on 28/05/2021.
//
#pragma once
#include <llvm/ADT/PointerUnion.h>

namespace lbc {
class CodeGen;
class Symbol;
struct AstExpr;
struct AstIdentExpr;

namespace Gen {
    namespace value_handler_detail {
        using ValuePtr = llvm::PointerIntPair<llvm::Value*, 1, bool>;
    } // namespace value_handler_detail

    class ValueHandler final : llvm::PointerUnion<value_handler_detail::ValuePtr, Symbol*, AstIdentExpr*> {
    public:
        /// Create temporary allocated variable - it is not inserted into symbol table
        static ValueHandler createTemp(CodeGen& gen, AstExpr& expr, StringRef name = "") noexcept;

        /// Create temporary variable if expression is not a constant
        static ValueHandler createTempOrConstant(CodeGen& gen, AstExpr& expr, StringRef name = "") noexcept;

        constexpr ValueHandler() noexcept = default;
        ValueHandler(CodeGen* gen, Symbol* symbol) noexcept;
        ValueHandler(CodeGen* gen, llvm::Value* value) noexcept;
        ValueHandler(CodeGen* gen, AstIdentExpr& ident) noexcept;

        [[nodiscard]] llvm::Value* load() noexcept;
        [[nodiscard]] llvm::Value* getAddress() noexcept;
        void store(llvm::Value* val) noexcept;
        void store(ValueHandler& val) noexcept {
            store(val.load());
        }

        [[nodiscard]] constexpr inline bool isValid() const noexcept {
            return m_gen != nullptr;
        }

    private:
        ValueHandler(CodeGen* gen, value_handler_detail::ValuePtr ptr) noexcept;

        CodeGen* m_gen = nullptr;
    };

} // namespace Gen
} // namespace lbc
