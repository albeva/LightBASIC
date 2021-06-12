//
// Created by Albert Varaksin on 06/07/2020.
//
#pragma once
#include "Symbol.hpp"

namespace lbc {
class Context;

class SymbolTable final {
    using Container = llvm::StringMap<Symbol*>;

public:
    NO_COPY_AND_MOVE(SymbolTable)
    explicit SymbolTable(SymbolTable* parent = nullptr) noexcept : m_parent{ parent } {}
    ~SymbolTable() noexcept = default;

    [[nodiscard]] SymbolTable* getParent() const noexcept { return m_parent; }
    void setParent(SymbolTable* parent) noexcept { m_parent = parent; }

    Symbol* insert(Context& context, StringRef name);
    void addReference(Symbol*);

    [[nodiscard]] bool exists(StringRef name, bool recursive = false) const noexcept;
    [[nodiscard]] Symbol* find(StringRef id, bool recursive = true) const noexcept;
    [[nodiscard]] std::vector<Symbol*> getSymbols() const;

    [[nodiscard]] auto size() const noexcept { return m_symbols.size(); }

    [[nodiscard]] auto begin() noexcept { return m_symbols.begin(); }
    [[nodiscard]] auto end() noexcept { return m_symbols.end(); }

    [[nodiscard]] auto begin() const noexcept { return m_symbols.begin(); }
    [[nodiscard]] auto end() const noexcept { return m_symbols.end(); }

private:
    SymbolTable* m_parent;
    Container m_symbols;
    llvm::StringMap<Symbol*> m_references;
};

} // namespace lbc
