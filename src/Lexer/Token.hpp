//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "pch.hpp"
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
    Memory
};

class Token final {
public:
    NO_COPY_AND_MOVE(Token)
    using Value = std::variant<std::monostate, StringRef, uint64_t, double, bool>;

    // Describe given token kind
    static StringRef description(TokenKind kind) noexcept;

    // find matching token for string or return TokenKind::Identifier
    static TokenKind findKind(StringRef str) noexcept;

    // constructors

    template<typename... Args>
    static unique_ptr<Token> create(Args&&... args) {
        return make_unique<Token>(std::forward<Args>(args)...);
    }

    Token(TokenKind kind, const llvm::SMRange& range)
    : m_kind{ kind }, m_value{ std::monostate{} }, m_range{ range } {}

    Token(TokenKind kind, StringRef value, const llvm::SMRange& range)
    : m_kind{ kind }, m_value{ value }, m_range{ range } {}

    Token(uint64_t value, const llvm::SMRange& range)
    : m_kind{ TokenKind::IntegerLiteral }, m_value{ value }, m_range{ range } {}

    Token(double value, const llvm::SMRange& range)
    : m_kind{ TokenKind::FloatingPointLiteral }, m_value{ value }, m_range{ range } {}

    Token(bool value, const llvm::SMRange& range)
    : m_kind{ TokenKind::BooleanLiteral }, m_value{ value }, m_range{ range } {}

    ~Token() noexcept = default;

    // convert token kind

    [[nodiscard]] unique_ptr<Token> convert(TokenKind kind) const {
        return create(kind, m_range);
    }

    // Getters

    [[nodiscard]] TokenKind kind() const noexcept { return m_kind; }

    [[nodiscard]] string lexeme() const;

    [[nodiscard]] const Value& getValue() const noexcept {
        return m_value;
    }

    [[nodiscard]] const llvm::SMRange& range() const noexcept {
        return m_range;
    };

    [[nodiscard]] StringRef description() const noexcept {
        return description(m_kind);
    }

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

    [[nodiscard]] bool operator==(TokenKind rhs) const noexcept {
        return m_kind == rhs;
    }
    [[nodiscard]] bool operator!=(TokenKind rhs) const noexcept {
        return m_kind != rhs;
    }

private:
    const TokenKind m_kind;
    const Value m_value;
    const llvm::SMRange m_range;
};

} // namespace lbc
