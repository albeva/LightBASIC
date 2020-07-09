//
// Created by Albert on 06/07/2020.
//
#include "SymbolTable.h"
#include "Symbol.h"
using namespace lbc;

Symbol* SymbolTable::insert(unique_ptr<Symbol>&& symbol) {
    return m_symbols.emplace(symbol->name(), std::move(symbol)).first->second.get();
}

bool SymbolTable::exists(const string_view& name, bool recursive) const {
    if (m_symbols.find(name) != m_symbols.end()) {
        return true;
    }

    return recursive && m_parent != nullptr && m_parent->exists(name, recursive);
}

Symbol *SymbolTable::find(const string_view& id, bool recursive) const {
    if (auto iter = m_symbols.find(id); iter != m_symbols.end()) {
        return iter->second.get();
    }

    if (recursive && m_parent != nullptr) {
        return m_parent->find(id, true);
    }

    return nullptr;
}
