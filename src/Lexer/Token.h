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

class Token final : noncopyable {
public:
    static unique_ptr<Token> create(const string_view& lexeme, const llvm::SMLoc& loc);

    static unique_ptr<Token> create(TokenKind kind, const string_view& lexeme, const llvm::SMLoc& loc);

    static const string_view& description(TokenKind kind);

    Token(TokenKind kind, const string_view& lexeme, const llvm::SMLoc& loc)
        : m_kind{kind}, m_lexeme{lexeme}, m_loc{loc} {}

    ~Token();

    inline TokenKind kind() const {
        return m_kind;
    }

    inline const string_view& lexeme() const {
        return m_lexeme;
    }

    inline const llvm::SMLoc& loc() const {
        return m_loc;
    }

    llvm::SMRange range() const;

    const string_view& description() const;

    bool isGeneral() const;

    bool isLiteral() const;

    bool isSymbol() const;

    bool isOperator() const;

    bool isKeyword() const;

    int getPrecedence() const;

    bool isBinary() const;

    bool isUnary() const;

    bool isLeftToRight() const;

    bool isRightToLeft() const;

    inline bool operator== (TokenKind rhs) const {
        return m_kind == rhs;
    }

    inline bool operator!= (TokenKind rhs) const {
        return m_kind != rhs;
    }

private:
    const TokenKind m_kind;
    const string_view m_lexeme;
    const llvm::SMLoc m_loc;
};

} // namespace lbc
