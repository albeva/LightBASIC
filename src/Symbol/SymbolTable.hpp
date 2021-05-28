//
// Created by Albert Varaksin on 06/07/2020.
//
#pragma once
#include "Symbol.hpp"
#include "pch.hpp"

namespace lbc {

class SymbolTable final {
public:
    NO_COPY_AND_MOVE(SymbolTable)

    explicit SymbolTable(SymbolTable* parent = nullptr) noexcept : m_parent{ parent } {}
    ~SymbolTable() noexcept = default;

    [[nodiscard]] SymbolTable* parent() const noexcept { return m_parent; }

    Symbol* insert(StringRef name) noexcept;
    void addReference(Symbol*) noexcept;

    [[nodiscard]] bool exists(StringRef name, bool recursive = false) const noexcept;
    [[nodiscard]] Symbol* find(StringRef id, bool recursive = true) const noexcept;

private:
    SymbolTable* m_parent;
    llvm::StringMap<unique_ptr<Symbol>> m_symbols;
    llvm::StringMap<Symbol*> m_references;
};

} // namespace lbc
