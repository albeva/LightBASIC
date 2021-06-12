//
// Created by Albert Varaksin on 06/07/2020.
//
#include "SymbolTable.hpp"
#include "Driver/Context.hpp"
#include "Symbol.hpp"
using namespace lbc;

Symbol* SymbolTable::insert(Context& context, StringRef name) {
    auto* symbol = context.create<Symbol>(name);
    symbol->setIndex(m_symbols.size());
    return m_symbols.insert({ name, symbol }).first->second;
}

void SymbolTable::addReference(Symbol* symbol) {
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
        return iter->second;
    }

    if (auto iter = m_references.find(id); iter != m_references.end()) {
        return iter->second;
    }

    if (recursive && m_parent != nullptr) {
        return m_parent->find(id, true);
    }

    return nullptr;
}

std::vector<Symbol*> SymbolTable::getSymbols() const {
    std::vector<Symbol*> symbols;
    symbols.reserve(size());

    std::transform(begin(), end(), std::back_inserter(symbols), [](const auto& item) {
        return item.second;
    });

    std::sort(symbols.begin(), symbols.end(), [](auto lhs, auto rhs) {
        return lhs->getIndex() < rhs->getIndex();
    });

    return symbols;
}
