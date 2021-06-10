//
// Created by Albert Varaksin on 03/07/2020.
//
#if defined(__CLION_IDE__)
#    pragma ide diagnostic ignored "cppcoreguidelines-pro-bounds-pointer-arithmetic"
#endif

#include "Lexer.hpp"
#include "Driver/Context.hpp"
#include "Token.hpp"
#include <charconv>
using namespace lbc;

namespace {
using llvm::isAlpha;
using llvm::isDigit;

inline bool isIdentifierChar(char ch) noexcept {
    return isAlpha(ch) || isDigit(ch) || ch == '_';
}

inline llvm::SMRange makeRange(const char* start, const char* end) noexcept {
    return { llvm::SMLoc::getFromPointer(start), llvm::SMLoc::getFromPointer(end) };
}
} // namespace

Lexer::Lexer(Context& context, unsigned fileID) noexcept
: m_context{ context },
  m_buffer{ m_context.getSourceMrg().getMemoryBuffer(fileID) },
  m_input{ m_buffer->getBufferStart() },
  m_eolPos{ m_input },
  m_hasStmt{ false } {}

void Lexer::next(Token& result) {
    // clang-format off
    while (true) {
        switch (*m_input) {
        case 0:
            return endOfFile(result);
        case '\r':
            m_eolPos = m_input;
            m_input++;
            if (*m_input == '\n') { // CR+LF
                m_input++;
            }
            if (m_hasStmt) {
                return endOfStatement(result);
            }
            continue;
        case '\n':
            m_eolPos = m_input;
            m_input++;
            if (m_hasStmt) {
                return endOfStatement(result);
            }
            continue;
        case '\t': case '\v': case '\f': case ' ':
            m_input++;
            continue;
        case '\'':
            skipUntilLineEnd();
            continue;
        case '/':
            if (m_input[1] == '\'') {
                skipMultilineComment();
                continue;
            }
            return token(result, TokenKind::Divide);
        case '_':
            if (isIdentifierChar(m_input[1])) {
                return identifier(result);
            }
            skipToNextLine();
            continue;
        case '"':
            return stringLiteral(result);
        case '=':
            return token(result, TokenKind::Assign);
        case ',':
            return token(result, TokenKind::Comma);
        case '.': {
            auto next = m_input[1];
            if (next == '.' && m_input[2] == '.') {
                return token(result, TokenKind::Ellipsis, 3);
            }
            if (isDigit(next)) {
                return numberLiteral(result);
            }
            return token(result, TokenKind::MemberAccess);
        }
        case '(':
            return token(result, TokenKind::ParenOpen);
        case ')':
            return token(result, TokenKind::ParenClose);
        case '[':
            return token(result, TokenKind::BracketOpen);
        case ']':
            return token(result, TokenKind::BracketClose);
        case '+':
            return token(result, TokenKind::Plus);
        case '-':
            return token(result, TokenKind::Minus);
        case '*':
            return token(result, TokenKind::Multiply);
        case '<': {
            auto la = m_input[1];
            if (la == '>') {
                return token(result, TokenKind::NotEqual, 2);
            }
            if (la == '=') {
                return token(result, TokenKind::LessOrEqual, 2);
            }
            return token(result, TokenKind::LessThan);
        }
        case '>':
            if (m_input[1] == '=') {
                return token(result, TokenKind::GreaterOrEqual, 2);
            }
            return token(result, TokenKind::GreaterThan);
        case '@':
            return token(result, TokenKind::AddressOf);
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return numberLiteral(result);
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
            return identifier(result);
        }

        return invalid(result, m_input);
    }
    // clang-format on
}

void Lexer::peek(Token& result) {
    const auto* input = m_input;
    const char* eolPos = m_eolPos;
    bool hasStmt = m_hasStmt;

    next(result);

    m_input = input;
    m_eolPos = eolPos;
    m_hasStmt = hasStmt;
}

void Lexer::skipUntilLineEnd() noexcept {
    // assume m_input[0] != \r || \n
    while (true) {
        switch (*++m_input) {
        case 0:
        case '\r':
        case '\n':
            return;
        default:
            continue;
        }
    }
}

void Lexer::skipToNextLine() noexcept {
    // assume m_input != \r || \n
    skipUntilLineEnd();
    switch (*m_input) {
    case '\r':
        m_input++;
        if (*m_input == '\n') { // CR+LF
            m_input++;
        }
        return;
    case '\n':
        m_input++;
        return;
    }
}

void Lexer::skipMultilineComment() noexcept {
    // assume m_input[0] == '/' && m_input[1] == '\''
    m_input++;
    int level = 1;
    while (true) {
        switch (*++m_input) {
        case '\0':
            return;
        case '\'':
            if (m_input[1] == '/') {
                m_input++;
                level--;
                if (level == 0) {
                    m_input++;
                    return;
                }
            }
            continue;
        case '/':
            if (m_input[1] == '\'') {
                m_input++;
                level++;
            }
        }
    }
}

void Lexer::endOfFile(Token& result) noexcept {
    if (m_hasStmt) {
        m_eolPos = m_input;
        return endOfStatement(result);
    }
    result.set(TokenKind::EndOfFile, makeRange(m_input, m_input));
}

void Lexer::endOfStatement(Token& result) noexcept {
    m_hasStmt = false;
    result.set(TokenKind::EndOfStmt, makeRange(m_eolPos, m_input));
}

void Lexer::invalid(Token& result, const char* loc) const noexcept {
    return result.set(TokenKind::Invalid, makeRange(loc, m_input));
}

void Lexer::stringLiteral(Token& result) {
    // assume m_input[0] == '"'
    m_hasStmt = true;
    const auto* start = m_input;

    string literal;
    const auto* begin = m_input + 1;
    while (true) {
        auto ch = *++m_input;
        switch (ch) {
        case '\t':
            continue;
        case '\\':
            if (begin < m_input) {
                literal.append(begin, m_input);
            }
            literal += escape();
            begin = m_input + 1;
            continue;
        case '"':
            if (begin < m_input) {
                literal.append(begin, m_input);
            }
            m_input++;
            break;
        default:
            constexpr char visibleFrom = 32;
            if (ch < visibleFrom) {
                return invalid(result, start);
            }
            continue;
        }
        break;
    }

    result.set(
        TokenKind::StringLiteral,
        makeRange(start, m_input),
        m_context.retainCopy(literal));
}

char Lexer::escape() noexcept {
    // assume m_input[0] == '\\'
    switch (*++m_input) {
    case 'a':
        return '\a';
    case 'b':
        return '\b';
    case 'f':
        return '\f';
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case 'v':
        return '\v';
    case '\\':
        return '\\';
    case '\'':
        return '\'';
    case '"':
        return '"';
    default:
        m_input--;
        return '\\';
    }
}

void Lexer::token(Token& result, TokenKind kind, int len) noexcept {
    // assume m_input[0] == op[0], m_input[len] == next ch
    m_hasStmt = true;
    const auto* start = m_input;
    m_input += len;
    result.set(kind, makeRange(start, m_input));
}

void Lexer::numberLiteral(Token& result) {
    // assume m_input[0] == '.' digit || digit
    m_hasStmt = true;
    const auto* start = m_input;

    bool isFloatingPoint = *m_input == '.';
    if (isFloatingPoint) {
        m_input++;
    }

    while (true) {
        auto ch = *++m_input;
        if (ch == '.') {
            if (isFloatingPoint) {
                return invalid(result, m_input);
            }
            isFloatingPoint = true;
            continue;
        }
        if (isDigit(ch)) {
            continue;
        }
        break;
    }

    if (isFloatingPoint) {
        std::string number{ start, m_input };
        std::size_t size{};
        double value = std::stod(number, &size);
        if (size == 0) {
            return invalid(result, start);
        }
        result.set(TokenKind::FloatingPointLiteral, makeRange(start, m_input), value);
        return;
    }

    uint64_t value{};
    constexpr int base10 = 10;
    if (std::from_chars(start, m_input, value, base10).ec != std::errc()) {
        return invalid(result, start);
    }
    result.set(TokenKind::IntegerLiteral, makeRange(start, m_input), value);
}

void Lexer::identifier(Token& result) {
    // assume m_input[0] == '_' || char
    m_hasStmt = true;
    const auto* start = m_input;

    while (isIdentifierChar(*++m_input)) {}

    std::string uppercased;
    auto length = std::distance(start, m_input);
    uppercased.reserve(static_cast<size_t>(length));
    std::transform(start, m_input, std::back_inserter(uppercased), llvm::toUpper);

    auto range = makeRange(start, m_input);
    auto kind = Token::findKind(uppercased);
    switch (kind) {
    case TokenKind::True:
        return result.set(TokenKind::BooleanLiteral, range, true);
    case TokenKind::False:
        return result.set(TokenKind::BooleanLiteral, range, false);
    case TokenKind::Null:
        return result.set(TokenKind::NullLiteral, range);
    case TokenKind::Identifier:
        return result.set(kind, range, m_context.retainCopy(uppercased));
    default:
        return result.set(kind, range);
    }
}
