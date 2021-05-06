//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "pch.h"
#include "Token.def.h"

namespace lbc {

enum class TokenKind {
#define IMPL_TOKENS(id, ...) id,
    ALL_TOKENS(IMPL_TOKENS)
#undef IMPL_TOKENS
};

class Token final {
public:
    NO_COPY_AND_MOVE(Token)

    /**
     * Create either identifier or a keyword token from string literal
     */
    static unique_ptr<Token> create(const StringRef& lexeme, const llvm::SMLoc& loc);

    /**
     * Create token with given kind and lexeme
     */
    static unique_ptr<Token> create(TokenKind kind, const StringRef& lexeme, const llvm::SMLoc& loc);

    /**
     * Create token with given kind and use description for lexeme
     */
    static unique_ptr<Token> create(TokenKind kind, const llvm::SMLoc& loc);

    /**
     * Get TokenKind string representation
     */
    static const StringRef& description(TokenKind kind);

    Token(TokenKind kind, const StringRef& lexeme, const llvm::SMLoc& loc)
    : m_kind{ kind }, m_lexeme{ lexeme }, m_loc{ loc } {}

    ~Token() = default;

    [[nodiscard]] unique_ptr<Token> map(TokenKind kind) const noexcept {
        return create(kind, m_loc);
    }

    [[nodiscard]] TokenKind kind() const { return m_kind; }
    [[nodiscard]] const StringRef& lexeme() const { return m_lexeme; }

    [[nodiscard]] uint64_t getIntegral() const noexcept;
    [[nodiscard]] double getDouble() const noexcept;
    [[nodiscard]] bool getBool() const noexcept;

    [[nodiscard]] inline const llvm::SMLoc& loc() const { return m_loc; }
    [[nodiscard]] llvm::SMRange range() const;

    [[nodiscard]] const StringRef& description() const;

    [[nodiscard]] bool isGeneral() const;
    [[nodiscard]] bool isLiteral() const;
    [[nodiscard]] bool isSymbol() const;
    [[nodiscard]] bool isOperator() const;
    [[nodiscard]] bool isKeyword() const;

    [[nodiscard]] int getPrecedence() const;
    [[nodiscard]] bool isBinary() const;
    [[nodiscard]] bool isUnary() const;
    [[nodiscard]] bool isLeftToRight() const;
    [[nodiscard]] bool isRightToLeft() const;

    inline bool operator==(TokenKind rhs) const {
        return m_kind == rhs;
    }

    inline bool operator!=(TokenKind rhs) const {
        return m_kind != rhs;
    }

private:
    const TokenKind m_kind;
    const StringRef m_lexeme;
    const llvm::SMLoc m_loc;
};

} // namespace lbc
