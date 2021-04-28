//
// Created by Albert Varaksin on 06/07/2020.
//
#pragma once
#include "pch.h"
#include "Symbol.h"

namespace lbc {

class SymbolTable final : private NonCopyable {
public:
    using Storage = llvm::StringMap<unique_ptr<Symbol>>;
    using iterator = Storage::iterator;

    explicit SymbolTable(SymbolTable* parent = nullptr) : m_parent{ parent } {}

    iterator begin() { return m_symbols.begin(); }
    iterator end() { return m_symbols.end(); }

    [[nodiscard]] SymbolTable* parent() const { return m_parent; }

    Symbol* insert(const llvm::StringRef& name);

    [[nodiscard]] bool exists(const llvm::StringRef& name, bool recursive = false) const;
    [[nodiscard]] Symbol* find(const llvm::StringRef& id, bool recursive = true) const;

private:
    SymbolTable* m_parent;
    Storage m_symbols;
};

} // namespace lbc
