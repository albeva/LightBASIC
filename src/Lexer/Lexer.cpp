//
// Created by Albert Varaksin on 03/07/2020.
//
#include "Lexer.h"
#include "Driver/Context.h"
#include "Token.h"
#include <charconv>
using namespace lbc;

namespace {
using llvm::isAlpha;
using llvm::isDigit;

inline bool isWhiteSpace(char ch) noexcept {
    return ch == ' ' || ch == '\t' || ch == '\f' || ch == '\v';
}

inline bool isLineEnd(char ch) noexcept {
    return ch == '\n';
}

inline llvm::SMRange getRange(const char* start, const char* end) noexcept {
    return { llvm::SMLoc::getFromPointer(start), llvm::SMLoc::getFromPointer(end) };
}

inline bool isIdentifierChar(char ch) noexcept {
    return isAlpha(ch) || isDigit(ch) || ch == '_';
}
} // namespace

Lexer::Lexer(Context& context, unsigned fileID) noexcept
: m_context{ context },
  m_buffer{ m_context.getSourceMrg().getMemoryBuffer(fileID) },
  m_input{ m_buffer->getBufferStart() },
  m_char{ *m_input },
  m_hasStmt{ false } {
    handleLineEnd();
}

unique_ptr<Token> Lexer::next() noexcept { // NOLINT
    while (isValid()) {
        // skip spaces
        if (isWhiteSpace(m_char)) {
            move();
            continue;
        }

        // new line, emit statement if there is one
        if (isLineEnd(m_char)) {
            if (m_hasStmt) {
                m_hasStmt = false;
                return endOfStatement();
            }
            move();
            continue;
        }

        // single line comments
        if (m_char == '\'') {
            skipUntilLineEnd();
            continue;
        }

        // multi line comments
        if (m_char == '/' && peek() == '\'') {
            multilineComment();
            continue;
        }

        // line continuation
        if (m_char == '_') {
            if (isIdentifierChar(peek())) {
                m_hasStmt = true;
                return identifier();
            }
            skipUntilLineEnd();
            if (m_char == '\n') {
                move();
            }
            continue;
        }

        // there is some parsable content. so set stmt to true
        m_hasStmt = true;

        // identifier
        if (isAlpha(m_char)) {
            return identifier();
        }

        // number
        if (isDigit(m_char) || (m_char == '.' && isDigit(peek()))) {
            return number();
        }

        switch (m_char) {
        case '"':
            return string();
        case '=':
            return op(TokenKind::Assign);
        case ',':
            return op(TokenKind::Comma);
        case '.':
            if (peek(1) == '.' && peek(2) == '.') {
                return ellipsis();
            }
            return op(TokenKind::Period);
        case '(':
            return op(TokenKind::ParenOpen);
        case ')':
            return op(TokenKind::ParenClose);
        case '[':
            return op(TokenKind::BracketOpen);
        case ']':
            return op(TokenKind::BracketClose);
        case '+':
            return op(TokenKind::Plus);
        case '-':
            return op(TokenKind::Minus);
        case '*':
            return op(TokenKind::Multiply);
        case '/':
            return op(TokenKind::Divide);
        case '<': {
            auto la = peek(1);
            if (la == '>') {
                move();
                return op(TokenKind::NotEqual);
            }
            if (la == '=') {
                move();
                return op(TokenKind::LessOrEqual);
            }
            return op(TokenKind::LessThan);
        }
        case '>':
            if (peek(1) == '=') {
                move();
                return op(TokenKind::GreaterOrEqual);
            }
            return op(TokenKind::GreaterThan);
        case '@':
            return op(TokenKind::AddressOf);
        default:
            return invalid(m_input);
        }
    }

    return endOfFile(); // NOLINT
}
/**
 * If `m_hasStmt` is true then return `EndOfLine`
 * before `EndOfFile`
 */
unique_ptr<Token> Lexer::endOfFile() noexcept {
    if (m_hasStmt) {
        m_hasStmt = false;
        return Token::create(
            TokenKind::EndOfStmt,
            getRange(m_buffer->getBufferEnd(), m_buffer->getBufferEnd()));
    }

    return Token::create(
        TokenKind::EndOfFile,
        getRange(m_buffer->getBufferEnd(), m_buffer->getBufferEnd()));
}

unique_ptr<Token> Lexer::endOfStatement() noexcept {
    const auto* start = m_input;
    move();
    return Token::create(TokenKind::EndOfStmt, getRange(start, m_input));
}

unique_ptr<Token> Lexer::ellipsis() noexcept {
    const auto* start = m_input;
    move(3);
    return Token::create(TokenKind::Ellipsis, getRange(start, m_input));
}

unique_ptr<Token> Lexer::identifier() noexcept {
    const auto* start = m_input;
    size_t length = 1;
    while (move() && isIdentifierChar(m_char)) {
        length++;
    }
    const auto* end = start + length; // NOLINT
    auto range = getRange(start, m_input);

    std::string uppercased;
    uppercased.reserve(length);
    std::transform(start, end, std::back_inserter(uppercased), llvm::toUpper);

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

unique_ptr<Token> Lexer::number() noexcept {
    bool isFloatingPoint = m_char == '.';

    const auto* start = m_input;
    size_t len = 1;
    while (move()) {
        if (m_char == '.') {
            if (isFloatingPoint) {
                return invalid(m_input);
            }
            isFloatingPoint = true;
        } else if (!isDigit(m_char)) {
            break;
        }
        len++;
    }
    const char* end = start + len; // NOLINT
    auto range = getRange(start, end);

    if (isFloatingPoint) {
        std::string number{ start, end };
        std::size_t size{};
        double value = std::stod(number, &size);
        if (size == 0) {
            return invalid(start);
        }
        return Token::create(value, range);
    }

    uint64_t value{};
    const int base10 = 10;
    if (std::from_chars(start, end, value, base10).ec != std::errc()) {
        return invalid(start);
    }
    return Token::create(value, range);
}

unique_ptr<Token> Lexer::string() noexcept {
    constexpr char visibleFrom = 32;
    const auto* start = m_input;
    std::string literal{};

    while (move() && m_char >= visibleFrom && m_char != '"') {
        if (m_char == '\\') {
            switch (peek()) {
            case '\\':
            case '"':
                move();
                literal += m_char;
                continue;
            case 'n':
                literal += '\n';
                move();
                continue;
            case 't':
                literal += '\t';
                move();
                continue;
            default:
                return invalid(m_input);
            }
        }
        literal += m_char;
    }

    if (m_char != '"') {
        return invalid(start);
    }
    move();

    return Token::create(
        TokenKind::StringLiteral,
        m_context.retainCopy(literal),
        getRange(start, m_input));
}

unique_ptr<Token> Lexer::op(TokenKind kind) noexcept {
    const auto* start = m_input;
    move();
    return Token::create(kind, getRange(start, m_input));
}

unique_ptr<Token> Lexer::invalid(const char* loc) noexcept {
    return Token::create(TokenKind::Invalid, getRange(loc, loc));
}

void Lexer::skipUntilLineEnd() noexcept {
    while (move() && m_char != '\n') {}
}

void Lexer::multilineComment() noexcept {
    move();
    int level = 1;
    while (move()) {
        if (m_char == '\'' && peek() == '/') {
            move(2);
            level--;
            if (level == 0) {
                return;
            }
        } else if (m_char == '/' && peek() == '\'') {
            move();
            level++;
        }
    }
}

bool Lexer::move() noexcept {
    m_char = *++m_input; // NOLINT
    handleLineEnd();
    return isValid();
}

bool Lexer::move(int steps) noexcept {
    while (steps-- > 0 && move()) {}
    return isValid();
}

void Lexer::handleLineEnd() noexcept {
    if (m_char == '\r') {
        if (peek() == '\n') {
            m_input++; // NOLINT
        }
        m_char = '\n';
    }
}

bool Lexer::isValid() const noexcept {
    return m_char != 0;
}

char Lexer::peek(int ahead) const noexcept {
    const auto* next = m_input + ahead; // NOLINT
    if (next < m_buffer->getBufferEnd()) {
        return *next;
    }
    return 0;
}
