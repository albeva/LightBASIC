//
// Created by Albert on 29/05/2021.
//
#pragma once
#include "pch.hpp"
#include "Type.hpp"

namespace lbc {
class Symbol;
class SymbolTable;

/**
 * User defined type
 */
class TypeUDT final : public TypeRoot {
    TypeUDT(Symbol& symbol, SymbolTable& symbolTable) noexcept;

public:
    static const TypeUDT* get(Symbol& symbol, SymbolTable& symbolTable) noexcept;

    constexpr static bool classof(const TypeRoot* type) noexcept {
        return type->getKind() == TypeFamily::UDT;
    }

    string asString() const noexcept final;

protected:
    llvm::Type* genLlvmType(Context& context) const noexcept override;

private:
    Symbol& m_symbol;
    SymbolTable& m_symbolTable;
};

} // namespace lbc
