//
// Created by Albert Varaksin on 06/07/2020.
//
#pragma once
#include "pch.hpp"
#include "Symbol.hpp"

namespace lbc {

class SymbolTable final {
    using Container = llvm::StringMap<unique_ptr<Symbol>>;

public:
    using iterator = Container::iterator;
    using const_iterator = Container::const_iterator;

    NO_COPY_AND_MOVE(SymbolTable)
    explicit SymbolTable(SymbolTable* parent = nullptr) noexcept : m_parent{ parent } {}
    ~SymbolTable() noexcept = default;

    [[nodiscard]] SymbolTable* parent() const noexcept { return m_parent; }

    Symbol* insert(StringRef name);
    void addReference(Symbol*);

    [[nodiscard]] bool exists(StringRef name, bool recursive = false) const noexcept;
    [[nodiscard]] Symbol* find(StringRef id, bool recursive = true) const noexcept;

    [[nodiscard]] iterator begin() noexcept { return m_symbols.begin(); }
    [[nodiscard]] iterator end() noexcept { return m_symbols.end(); }

    [[nodiscard]] const_iterator begin() const noexcept { return m_symbols.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return m_symbols.end(); }

private:
    SymbolTable* m_parent;
    Container m_symbols;
    llvm::StringMap<Symbol*> m_references;
};

} // namespace lbc
