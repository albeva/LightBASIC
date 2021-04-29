//
// Created by Albert Varaksin on 06/07/2020.
//
#pragma once
#include "pch.h"

namespace lbc {

class TypeRoot;

class Symbol final : private NonCopyable {
public:
    explicit Symbol(const StringRef& name, const TypeRoot* type = nullptr)
    : m_name{ name }, m_type{ type }, m_alias{ "" } {}

    [[nodiscard]] const TypeRoot* type() const { return m_type; }
    void setType(const TypeRoot* type) { m_type = type; }

    [[nodiscard]] const StringRef& name() const { return m_name; }

    [[nodiscard]] llvm::Value* value() const { return m_value; }
    void setValue(llvm::Value* value) { m_value = value; }

    [[nodiscard]] const StringRef& alias() const { return m_alias; }
    void setAlias(const StringRef& alias) { m_alias = alias; }

    [[nodiscard]] const StringRef& identifier() const {
        if (m_alias.empty()) {
            return m_name;
        }
        return m_alias;
    }

private:
    const StringRef m_name;
    const TypeRoot* m_type;

    StringRef m_alias;
    llvm::Value* m_value = nullptr;
};

} // namespace lbc
