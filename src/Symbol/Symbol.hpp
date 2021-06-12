//
// Created by Albert Varaksin on 06/07/2020.
//
#pragma once
#include "Ast/ValueFlags.hpp"

namespace lbc {
class TypeRoot;

class Symbol final {
public:
    NO_COPY_AND_MOVE(Symbol)

    explicit Symbol(StringRef name, const TypeRoot* type = nullptr) noexcept
    : m_name{ name }, m_type{ type }, m_alias{ "" } {}

    ~Symbol() noexcept = default;

    [[nodiscard]] ValueFlags getFlags() const noexcept { return m_flags; }
    void setFlags(ValueFlags flags) noexcept { m_flags = flags; }

    [[nodiscard]] Symbol* getParent() const noexcept { return m_parent; }
    void setParent(Symbol* parent) noexcept { m_parent = parent; }

    [[nodiscard]] unsigned int getIndex() const noexcept { return m_index; }
    void setIndex(unsigned int index) noexcept { m_index = index; }

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
    Symbol* m_parent = nullptr;
    unsigned int m_index = 0;
    ValueFlags m_flags{};
};

} // namespace lbc
