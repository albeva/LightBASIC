//
// Created by Albert on 03/07/2020.
//
#include "Token.h"
using namespace lbc;

using std::unordered_set;

namespace {

namespace literals {
#define IMPL_LITERAL(id, kw, ...) constexpr string_view Str##id{ kw };
    ALL_TOKENS(IMPL_LITERAL)
    KEYWORD_TOKEN_MAP(IMPL_LITERAL)
#undef IMPL_LITERAL
} // namespace literals

/// Map string literal to TokenKind
const std::unordered_map<string_view, TokenKind> keywordsToKind {
#define IMPL_LITERAL(id, ...) { literals::Str##id, TokenKind::id },
    TOKEN_SYMBOLS(IMPL_LITERAL)
    TOKEN_OPERATORS(IMPL_LITERAL)
    TOKEN_KEYWORDS(IMPL_LITERAL)
    ALL_TYPES(IMPL_LITERAL)
#undef IMPL_LITERAL
#define IMPL_KEYWORD_MAP(id, kw, map) { literals::Str##id, TokenKind::map },
    KEYWORD_TOKEN_MAP(IMPL_KEYWORD_MAP)
#undef IMPL_KEYWORD_MAP
};

constexpr std::array kindToDescription {
#define IMPL_LITERAL(id, kw, ...) literals::Str##id,
    ALL_TOKENS(IMPL_LITERAL)
#undef IMPL_LITERAL
};

unordered_set<string> uppercasedIds;
unordered_set<string> literalStrings;

} // namespace

const string_view& Token::description(TokenKind kind) {
    auto index = static_cast<size_t>(kind);
    return kindToDescription.at(index);
}

unique_ptr<Token> Token::create(const string_view& lexeme, const llvm::SMLoc& loc) {
    // all identifiers in lb are upper cased
    string uppercased;
    std::transform(lexeme.cbegin(), lexeme.cend(), std::back_inserter(uppercased), llvm::toUpper);

    // is there a matching keyword?
    auto iter = keywordsToKind.find(uppercased);
    if (iter != keywordsToKind.end()) {
        return Token::create(iter->second, iter->first, loc);
    }

    // tokens store string_view instances, therefore we need to keep
    // actual string data around. Hence storing all identifiers in a set
    auto entry = uppercasedIds.insert(std::move(uppercased));
    return Token::create(TokenKind::Identifier, *entry.first, loc);
}

unique_ptr<Token> Token::create(TokenKind kind, const string_view& lexeme, const llvm::SMLoc& loc) {
    // literal string may have been processed for escape sequences. Store a local copy
    if (kind == TokenKind::StringLiteral) {
        auto entry = literalStrings.insert(string{lexeme});
        return make_unique<Token>(kind, *entry.first, loc);
    }
    return make_unique<Token>(kind, lexeme, loc);
}

unique_ptr<Token> Token::create(TokenKind kind, const llvm::SMLoc& loc) {
    return create(kind, description(kind), loc);
}

llvm::SMRange Token::range() const {
    if (isGeneral()) {
        return llvm::None;
    }

    auto size = static_cast<ptrdiff_t>(m_lexeme.size());
    if (m_kind == TokenKind::StringLiteral) {
        size += 2;
    }

    const auto* end = m_loc.getPointer() + size; // NOLINT
    return { m_loc, llvm::SMLoc::getFromPointer(end) };
}

const string_view& Token::description() const {
    if (m_kind == TokenKind::Identifier) {
        return m_lexeme;
    }
    auto index = static_cast<size_t>(m_kind);
    return kindToDescription.at(index);
}

bool Token::isGeneral() const {
#define CASE_GENERAL(id, ...) case TokenKind::id:
    switch (m_kind) {
        TOKEN_GENERAL(CASE_GENERAL)
        return true;
    default:
        return false;
    }
#undef CASE_LITERAL
}

bool Token::isLiteral() const {
#define CASE_LITERAL(id, ...) case TokenKind::id:
    switch (m_kind) {
        TOKEN_LITERALS(CASE_LITERAL)
        return true;
    default:
        return false;
    }
#undef CASE_LITERAL
}

bool Token::isSymbol() const {
#define CASE_SYMBOL(id, ...) case TokenKind::id:
    switch (m_kind) {
        TOKEN_SYMBOLS(CASE_SYMBOL)
        return true;
    default:
        return false;
    }
#undef CASE_SYMBOL
}

bool Token::isOperator() const {
#define CASE_OPERATOR(id, ...) case TokenKind::id:
    switch (m_kind) {
        TOKEN_OPERATORS(CASE_OPERATOR)
        return true;
    default:
        return false;
    }
#undef CASE_OPERATOR
}

bool Token::isKeyword() const {
#define CASE_KEYWORD(id, ...) case TokenKind::id:
    switch (m_kind) {
        TOKEN_KEYWORDS(CASE_KEYWORD)
        return true;
    default:
        return false;
    }
#undef CASE_KEYWORD
}

int Token::getPrecedence() const {
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

bool Token::isBinary() const {
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

bool Token::isUnary() const {
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

bool Token::isLeftToRight() const {
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

bool Token::isRightToLeft() const {
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
