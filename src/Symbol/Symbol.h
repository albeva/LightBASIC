//
// Created by Albert Varaksin on 06/07/2020.
//
#pragma once
#include "pch.h"

namespace lbc {

class TypeRoot;

class Symbol final {
public:
    NO_COPY_AND_MOVE(Symbol)

    explicit Symbol(StringRef name, const TypeRoot* type = nullptr)
    : m_name{ name }, m_type{ type }, m_alias{ "" } {}

    ~Symbol() = default;

    [[nodiscard]] const TypeRoot* type() const { return m_type; }
    void setType(const TypeRoot* type) { m_type = type; }

    [[nodiscard]] bool isExternal() const { return m_external; }
    void setExternal(bool external) { m_external = external; }

    [[nodiscard]] StringRef name() const { return m_name; }

    [[nodiscard]] llvm::Value* getLlvmValue() const { return m_llvmValue; }
    void setLlvmValue(llvm::Value* value) { m_llvmValue = value; }

    [[nodiscard]] StringRef alias() const { return m_alias; }
    void setAlias(StringRef alias) { m_alias = alias; }

    [[nodiscard]] StringRef identifier() const {
        if (m_alias.empty()) {
            return m_name;
        }
        return m_alias;
    }

    [[nodiscard]] llvm::GlobalValue::LinkageTypes getLlvmLinkage() const {
        if (m_external) {
            return llvm::GlobalValue::LinkageTypes::ExternalLinkage;
        }
        return llvm::GlobalValue::LinkageTypes::InternalLinkage;
    }

private:
    const StringRef m_name;
    const TypeRoot* m_type;

    StringRef m_alias;
    llvm::Value* m_llvmValue = nullptr;
    bool m_external = false;
};

} // namespace lbc
