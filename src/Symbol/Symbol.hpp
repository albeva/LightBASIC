//
// Created by Albert Varaksin on 06/07/2020.
//
#pragma once
#include "pch.hpp"

namespace lbc {

class TypeRoot;

class Symbol final {
public:
    NO_COPY_AND_MOVE(Symbol)

    explicit Symbol(StringRef name, const TypeRoot* type = nullptr) noexcept
    : m_name{ name }, m_type{ type }, m_alias{ "" } {}

    ~Symbol() noexcept = default;

    [[nodiscard]] const TypeRoot* type() const noexcept { return m_type; }
    void setType(const TypeRoot* type) noexcept { m_type = type; }

    [[nodiscard]] bool isExternal() const noexcept { return m_external; }
    void setExternal(bool external) noexcept { m_external = external; }

    [[nodiscard]] StringRef name() const noexcept { return m_name; }

    [[nodiscard]] llvm::Value* getLlvmValue() const noexcept { return m_llvmValue; }
    void setLlvmValue(llvm::Value* value) noexcept { m_llvmValue = value; }

    [[nodiscard]] StringRef alias() const noexcept { return m_alias; }
    void setAlias(StringRef alias) noexcept { m_alias = alias; }

    [[nodiscard]] StringRef identifier() const noexcept {
        if (m_alias.empty()) {
            return m_name;
        }
        return m_alias;
    }

    [[nodiscard]] llvm::GlobalValue::LinkageTypes getLlvmLinkage() const noexcept {
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
