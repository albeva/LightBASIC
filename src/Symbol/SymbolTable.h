//
// Created by Albert on 06/07/2020.
//
#pragma once
#include "pch.h"

namespace lbc {

class Symbol;

class SymbolTable final: noncopyable {
public:
    using Storage = std::unordered_map<string_view, unique_ptr<Symbol>>;
    using iterator = Storage::iterator;

    SymbolTable(SymbolTable * parent = nullptr);
    ~SymbolTable();

    iterator begin() { return m_symbols.begin(); }
    iterator end() { return m_symbols.end(); }

    SymbolTable * parent() const { return m_parent; }

    Symbol * insert(unique_ptr<Symbol>&& symbol);

    bool exists(const string_view& name, bool recursive = false);
    Symbol* find(const string_view& id, bool recursive = true);

private:
    SymbolTable* m_parent;
    Storage m_symbols;
};

} // namespace lbc
