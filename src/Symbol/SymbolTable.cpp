//
// Created by Albert Varaksin on 06/07/2020.
//
#include "SymbolTable.hpp"
#include "Symbol.hpp"
using namespace lbc;

Symbol* SymbolTable::insert(StringRef name) noexcept {
    return m_symbols.insert({ name, make_unique<Symbol>(name) }).first->second.get();
}

void SymbolTable::addReference(Symbol* symbol) noexcept {
    m_references.insert({ symbol->name(), symbol });
}

bool SymbolTable::exists(StringRef name, bool recursive) const noexcept {
    if (m_symbols.find(name) != m_symbols.end()) {
        return true;
    }

    if (m_references.find(name) != m_references.end()) {
        return true;
    }

    return recursive && m_parent != nullptr && m_parent->exists(name, recursive);
}

Symbol* SymbolTable::find(StringRef id, bool recursive) const noexcept {
    if (auto iter = m_symbols.find(id); iter != m_symbols.end()) {
        return iter->second.get();
    }

    if (auto iter = m_references.find(id); iter != m_references.end()) {
        return iter->second;
    }

    if (recursive && m_parent != nullptr) {
        return m_parent->find(id, true);
    }

    return nullptr;
}
