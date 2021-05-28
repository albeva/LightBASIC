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

unique_ptr<Token> Lexer::next() noexcept {
    // clang-format off
    while (true) {
        switch (*m_input) {
        case 0:
            return endOfFile();
        case '\r':
            m_eolPos = m_input;
            m_input++;
            if (*m_input == '\n') { // CR+LF
                m_input++;
            }
            if (m_hasStmt) {
                return endOfStatement();
            }
            continue;
        case '\n':
            m_eolPos = m_input;
            m_input++;
            if (m_hasStmt) {
                return endOfStatement();
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
            return token(TokenKind::Divide);
        case '_':
            if (isIdentifierChar(m_input[1])) {
                return identifier();
            }
            skipToNextLine();
            continue;
        case '"':
            return stringLiteral();
        case '=':
            return token(TokenKind::Assign);
        case ',':
            return token(TokenKind::Comma);
        case '.': {
            auto next = m_input[1];
            if (next == '.' && m_input[2] == '.') {
                return token(TokenKind::Ellipsis, 3);
            }
            if (isDigit(next)) {
                return numberLiteral();
            }
            return token(TokenKind::Period);
        }
        case '(':
            return token(TokenKind::ParenOpen);
        case ')':
            return token(TokenKind::ParenClose);
        case '[':
            return token(TokenKind::BracketOpen);
        case ']':
            return token(TokenKind::BracketClose);
        case '+':
            return token(TokenKind::Plus);
        case '-':
            return token(TokenKind::Minus);
        case '*':
            return token(TokenKind::Multiply);
        case '<': {
            auto la = m_input[1];
            if (la == '>') {
                return token(TokenKind::NotEqual, 2);
            }
            if (la == '=') {
                return token(TokenKind::LessOrEqual, 2);
            }
            return token(TokenKind::LessThan);
        }
        case '>':
            if (m_input[1] == '=') {
                return token(TokenKind::GreaterOrEqual, 2);
            }
            return token(TokenKind::GreaterThan);
        case '@':
            return token(TokenKind::AddressOf);
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            return numberLiteral();
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
        case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
        case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
        case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
        case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
        case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
        case 'v': case 'w': case 'x': case 'y': case 'z':
            return identifier();
        }

        return invalid(m_input);
    }
    // clang-format on
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

unique_ptr<Token> Lexer::endOfFile() noexcept {
    if (m_hasStmt) {
        m_eolPos = m_input;
        return endOfStatement();
    }
    return Token::create(TokenKind::EndOfFile, makeRange(m_input, m_input));
}

unique_ptr<Token> Lexer::endOfStatement() noexcept {
    m_hasStmt = false;
    return Token::create(TokenKind::EndOfStmt, makeRange(m_eolPos, m_input));
}

unique_ptr<Token> Lexer::invalid(const char* loc) const noexcept {
    return Token::create(TokenKind::Invalid, makeRange(loc, m_input));
}

unique_ptr<Token> Lexer::stringLiteral() noexcept {
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
                return invalid(start);
            }
            continue;
        }
        break;
    }

    return Token::create(
        TokenKind::StringLiteral,
        m_context.retainCopy(literal),
        makeRange(start, m_input));
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

unique_ptr<Token> Lexer::token(TokenKind kind, int len) noexcept {
    // assume m_input[0] == op[0], m_input[len] == next ch
    m_hasStmt = true;
    const auto* start = m_input;
    m_input += len;
    return Token::create(kind, makeRange(start, m_input));
}

unique_ptr<Token> Lexer::numberLiteral() noexcept {
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
                return invalid(m_input);
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
            return invalid(start);
        }
        return Token::create(value, makeRange(start, m_input));
    }

    uint64_t value{};
    constexpr int base10 = 10;
    if (std::from_chars(start, m_input, value, base10).ec != std::errc()) {
        return invalid(start);
    }
    return Token::create(value, makeRange(start, m_input));
}


unique_ptr<Token> Lexer::identifier() noexcept {
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
        return Token::create(true, range);
    case TokenKind::False:
        return Token::create(false, range);
    case TokenKind::Null:
        return Token::create(TokenKind::NullLiteral, range);
    case TokenKind::Identifier:
        return Token::create(kind, m_context.retainCopy(uppercased), range);
    default:
        return Token::create(kind, range);
    }
}
