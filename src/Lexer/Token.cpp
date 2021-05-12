//
// Created by Albert Varaksin on 03/07/2020.
//
#include "Token.h"

using namespace lbc;

namespace {
    namespace literals {
#define IMPL_LITERAL(id, kw, ...) constexpr llvm::StringLiteral Str##id{ kw };
        ALL_TOKENS(IMPL_LITERAL)
#undef IMPL_LITERAL
    } // namespace literals

    // Map string literal to TokenKind
    const llvm::StringMap<TokenKind> keywordsToKind{
#define IMPL_LITERAL(id, ...) { literals::Str##id, TokenKind::id },
        TOKEN_KEYWORDS(IMPL_LITERAL)
        ALL_TYPES(IMPL_LITERAL)
        TOKEN_OPERAOTR_KEYWORD_MAP(IMPL_LITERAL)
#undef IMPL_LITERAL
    };

    constexpr std::array kindToDescription{
#define IMPL_LITERAL(id, kw, ...) literals::Str##id,
        ALL_TOKENS(IMPL_LITERAL)
#undef IMPL_LITERAL
    };

} // namespace

const StringRef &Token::description(TokenKind kind) noexcept {
    auto index = static_cast<size_t>(kind);
    return kindToDescription.at(index);
}

TokenKind Token::findKind(StringRef str) noexcept {
    auto iter = keywordsToKind.find(str);
    if (iter != keywordsToKind.end()) {
        return iter->second;
    }
    return TokenKind::Identifier;
}

bool Token::isGeneral() const noexcept {
#define CASE_GENERAL(id, ...) case TokenKind::id:
    switch (m_kind) {
        TOKEN_GENERAL(CASE_GENERAL)
            return true;
        default:
            return false;
    }
#undef CASE_LITERAL
}

bool Token::isLiteral() const noexcept {
#define CASE_LITERAL(id, ...) case TokenKind::id:
    switch (m_kind) {
        TOKEN_LITERALS(CASE_LITERAL)
            return true;
        default:
            return false;
    }
#undef CASE_LITERAL
}

bool Token::isSymbol() const noexcept {
#define CASE_SYMBOL(id, ...) case TokenKind::id:
    switch (m_kind) {
        TOKEN_SYMBOLS(CASE_SYMBOL)
            return true;
        default:
            return false;
    }
#undef CASE_SYMBOL
}

bool Token::isOperator() const noexcept {
#define CASE_OPERATOR(id, ...) case TokenKind::id:
    switch (m_kind) {
        TOKEN_OPERATORS(CASE_OPERATOR)
            return true;
        default:
            return false;
    }
#undef CASE_OPERATOR
}

bool Token::isKeyword() const noexcept {
#define CASE_KEYWORD(id, ...) case TokenKind::id:
    switch (m_kind) {
        TOKEN_KEYWORDS(CASE_KEYWORD)
            return true;
        default:
            return false;
    }
#undef CASE_KEYWORD
}

int Token::getPrecedence() const noexcept {
#define CASE_OPERATOR(id, ch, prec, ...) \
    case TokenKind::id:                  \
        return prec;
    switch (m_kind) {
        TOKEN_OPERATORS(CASE_OPERATOR) // NOLINT
        default:
            return 0;
    }
#undef CASE_OPERATOR
}

bool Token::isBinary() const noexcept {
    constexpr bool Binary = true; // NOLINT
    constexpr bool Unary = false; // NOLINT
#define CASE_OPERATOR(id, ch, prec, binary, ...) \
    case TokenKind::id:                          \
        return binary;
    switch (m_kind) {
        TOKEN_OPERATORS(CASE_OPERATOR) // NOLINT
        default:
            return false;
    }
#undef CASE_OPERATOR
}

bool Token::isUnary() const noexcept {
    constexpr bool Binary = false; // NOLINT
    constexpr bool Unary = true;   // NOLINT
#define CASE_OPERATOR(id, ch, prec, binary, ...) \
    case TokenKind::id:                          \
        return binary;
    switch (m_kind) {
        TOKEN_OPERATORS(CASE_OPERATOR) // NOLINT
        default:
            return false;
    }
#undef CASE_OPERATOR
}

bool Token::isLeftToRight() const noexcept {
    constexpr bool Left = true;  // NOLINT
    constexpr bool Right = true; // NOLINT
#define CASE_OPERATOR(id, ch, prec, binary, dir, ...) \
    case TokenKind::id:                               \
        return dir;
    switch (m_kind) {
        TOKEN_OPERATORS(CASE_OPERATOR) // NOLINT
        default:
            return false;
    }
#undef CASE_OPERATOR
}

bool Token::isRightToLeft() const noexcept {
    constexpr bool Left = false; // NOLINT
    constexpr bool Right = true; // NOLINT
#define CASE_OPERATOR(id, ch, prec, binary, dir, ...) \
    case TokenKind::id:                               \
        return dir;
    switch (m_kind) {
        TOKEN_OPERATORS(CASE_OPERATOR) // NOLINT
        default:
            return false;
    }
#undef CASE_OPERATOR
}

OperatorType Token::getOperatorType(TokenKind kind) noexcept {
#define CASE_OPERATOR(ID, CH, PREC, BINARY, DIR, KIND, ...) \
    case TokenKind::ID:                                     \
        return OperatorType::KIND;
    switch (kind) {
        TOKEN_OPERATORS(CASE_OPERATOR) // NOLINT
        default:
            llvm_unreachable("Unknown operator type");
    }
#undef CASE_OPERATOR
}
