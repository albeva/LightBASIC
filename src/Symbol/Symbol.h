//
// Created by Albert on 06/07/2020.
//
#pragma once
#include "pch.h"

namespace lbc {

class TypeRoot;

class Symbol final {
public:
    Symbol(const string_view& name, const TypeRoot* type)
    : m_name{name}, m_type{type}, m_alias{name} {}

    ~Symbol();

    const TypeRoot* type() const { return m_type; }
    const string_view& name() const { return m_name; }

    llvm::Value* value() const { return m_value; }
    void setValue(llvm::Value *value) { m_value = value; }

    const string_view& alias() const { return m_alias; }
    void setAlias(const string_view& alias) { m_alias = alias; }

private:
    const string_view m_name;
    const TypeRoot* m_type;

    string_view m_alias;
    llvm::Value* m_value;
};

} // namespace lbc
