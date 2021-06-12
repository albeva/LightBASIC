//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "Token.def.hpp"

namespace lbc {

enum class TokenKind {
#define IMPL_TOKENS(id, ...) id,
    ALL_TOKENS(IMPL_TOKENS)
#undef IMPL_TOKENS
};

enum class OperatorType {
    Arithmetic,
    Logical,
    Comparison,
    Memory,
    Assignment
};

class Token final {
public:
    using Value = std::variant<std::monostate, StringRef, uint64_t, double, bool>;

    // Describe given token kind
    static StringRef description(TokenKind kind) noexcept;

    // find matching token for string or return TokenKind::Identifier
    static TokenKind findKind(StringRef str) noexcept;

    // set token values
    void set(TokenKind kind, const llvm::SMRange& range, Value value = std::monostate{}) noexcept {
        m_kind = kind;
        m_range = range;
        m_value = value;
    }

    // Getters
    [[nodiscard]] TokenKind getKind() const noexcept { return m_kind; }
    void setKind(TokenKind kind) noexcept { m_kind = kind; }

    [[nodiscard]] StringRef lexeme() const noexcept;
    [[nodiscard]] string asString() const;
    [[nodiscard]] const Value& getValue() const noexcept { return m_value; }
    [[nodiscard]] StringRef getStringValue() const { return std::get<StringRef>(m_value); }
    [[nodiscard]] const llvm::SMRange& range() const noexcept { return m_range; };
    [[nodiscard]] StringRef description() const noexcept { return description(m_kind); }

    // Info about operators
    [[nodiscard]] bool isGeneral() const noexcept;
    [[nodiscard]] bool isLiteral() const noexcept;
    [[nodiscard]] bool isSymbol() const noexcept;
    [[nodiscard]] bool isOperator() const noexcept;
    [[nodiscard]] bool isKeyword() const noexcept;

    [[nodiscard]] int getPrecedence() const noexcept;
    [[nodiscard]] bool isBinary() const noexcept;
    [[nodiscard]] bool isUnary() const noexcept;
    [[nodiscard]] bool isLeftToRight() const noexcept;
    [[nodiscard]] bool isRightToLeft() const noexcept;

    static OperatorType getOperatorType(TokenKind kind) noexcept;

    // Query keyword types

    [[nodiscard]] bool isTypeKeyword() const noexcept;

    // comparisons

    [[nodiscard]] inline bool is(TokenKind kind) const noexcept {
        return m_kind == kind;
    }

    [[nodiscard]] inline bool isNot(TokenKind kind) const noexcept {
        return m_kind != kind;
    }

    [[nodiscard]] inline bool isOneOf(TokenKind k1, TokenKind k2) const noexcept {
        return is(k1) || is(k2);
    }

    template<typename... Ts>
    [[nodiscard]] inline bool isOneOf(TokenKind k1, TokenKind k2, Ts... ks) const noexcept {
        return is(k1) || isOneOf(k2, ks...);
    }

private:
    TokenKind m_kind = TokenKind::Invalid;
    Value m_value = std::monostate{};
    llvm::SMRange m_range{};
};

} // namespace lbc
