//
// Created by Albert Varaksin on 06/07/2020.
//
#pragma once
#include "pch.h"

namespace lbc {

class TypeRoot;

class Symbol final : private NonCopyable {
public:
    explicit Symbol(const string_view& name, const TypeRoot* type = nullptr)
    : m_name{ name }, m_type{ type }, m_alias{ name } {}

    [[nodiscard]] const TypeRoot* type() const { return m_type; }
    void setType(const TypeRoot* type) { m_type = type; }

    [[nodiscard]] const string_view& name() const { return m_name; }

    [[nodiscard]] llvm::Value* value() const { return m_value; }
    void setValue(llvm::Value* value) { m_value = value; }

    [[nodiscard]] const string_view& alias() const { return m_alias; }
    void setAlias(const string_view& alias) { m_alias = alias; }

private:
    const string_view m_name;
    const TypeRoot* m_type;

    string_view m_alias;
    llvm::Value* m_value = nullptr;
};

} // namespace lbc
