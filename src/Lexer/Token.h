//
// Created by Albert on 03/07/2020.
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
    NON_COPYABLE(Token)
public:
    ~Token() = default;

    static unique_ptr<Token> create(const string_view& lexeme, const llvm::SMLoc& loc);

    static unique_ptr<Token> create(TokenKind kind, const string_view& lexeme, const llvm::SMLoc& loc);

    static const string_view& description(TokenKind kind);

    Token(TokenKind kind, const string_view& lexeme, const llvm::SMLoc& loc)
        : m_kind{kind}, m_lexeme{lexeme}, m_loc{loc} {}

    [[nodiscard]] inline TokenKind kind() const { return m_kind; }
    [[nodiscard]] inline const string_view& lexeme() const { return m_lexeme; }

    [[nodiscard]] inline const llvm::SMLoc& loc() const { return m_loc; }
    [[nodiscard]] llvm::SMRange range() const;

    [[nodiscard]] const string_view& description() const;

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
    const string_view m_lexeme;
    const llvm::SMLoc m_loc;
};

} // namespace lbc
