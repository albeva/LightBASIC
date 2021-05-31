//
// Created by Albert on 29/05/2021.
//
#pragma once
#include "Type.hpp"

namespace lbc {
class Symbol;
class SymbolTable;

/**
 * User defined type
 */
class TypeUDT final : public TypeRoot {
    TypeUDT(Symbol& symbol, SymbolTable& symbolTable);

public:
    static const TypeUDT* get(Symbol& symbol, SymbolTable& symbolTable);

    constexpr static bool classof(const TypeRoot* type) {
        return type->getKind() == TypeFamily::UDT;
    }

    [[nodiscard]] string asString() const final;

    [[nodiscard]] Symbol& getSymbol() const noexcept { return m_symbol; }
    [[nodiscard]] SymbolTable& getSymbolTable() const noexcept { return m_symbolTable; }

protected:
    llvm::Type* genLlvmType(Context& context) const override;

private:
    Symbol& m_symbol;
    SymbolTable& m_symbolTable;
};

} // namespace lbc
