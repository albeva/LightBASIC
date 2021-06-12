//
// Created by Albert on 29/05/2021.
//
#pragma once
#include "Type.hpp"

namespace lbc {
class Symbol;
class SymbolTable;
class Context;

/**
 * User defined type
 */
class TypeUDT final : public TypeRoot {
    TypeUDT(Symbol& symbol, SymbolTable& symbolTable, bool packed);
    friend class Context;

public:
    static const TypeUDT* get(Context& context, Symbol& symbol, SymbolTable& symbolTable, bool packed);

    constexpr static bool classof(const TypeRoot* type) {
        return type->getKind() == TypeFamily::UDT;
    }

    [[nodiscard]] string asString() const final;

    [[nodiscard]] Symbol& getSymbol() const noexcept { return m_symbol; }
    [[nodiscard]] SymbolTable& getSymbolTable() const noexcept { return m_symbolTable; }

protected:
    llvm::Type* genLlvmType(Context& context) const final;

private:
    Symbol& m_symbol;
    SymbolTable& m_symbolTable;
    bool m_packed;
};

} // namespace lbc
