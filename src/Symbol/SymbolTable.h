//
// Created by Albert Varaksin on 06/07/2020.
//
#pragma once
#include "pch.h"

namespace lbc {

class Symbol;

class SymbolTable final: private NonCopyable {
public:
    using Storage = std::unordered_map<string_view, unique_ptr<Symbol>>;
    using iterator = Storage::iterator;

    explicit SymbolTable(SymbolTable* parent = nullptr) : m_parent{ parent } {}

    iterator begin() { return m_symbols.begin(); }
    iterator end() { return m_symbols.end(); }

    [[nodiscard]] SymbolTable* parent() const { return m_parent; }

    Symbol* insert(unique_ptr<Symbol>&& symbol);

    [[nodiscard]] bool exists(const string_view& name, bool recursive = false) const;
    [[nodiscard]] Symbol* find(const string_view& id, bool recursive = true) const;

private:
    SymbolTable* m_parent;
    Storage m_symbols;
};

} // namespace lbc
