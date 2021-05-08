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

inline bool isWhiteSpace(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\f' || ch == '\v';
}

inline bool isLineEnd(char ch) {
    return ch == '\n';
}

inline llvm::SMRange getRange(const char* start, const char* end) {
    return { llvm::SMLoc::getFromPointer(start), llvm::SMLoc::getFromPointer(end) };
}
} // namespace

Lexer::Lexer(Context& context, unsigned fileID)
: m_context{ context },
  m_fileID{ fileID },
  m_buffer{ m_context.getSourceMrg().getMemoryBuffer(fileID) },
  m_input{ m_buffer->getBufferStart() },
  m_char{ *m_input },
  m_hasStmt{ false } {
    handleLineEnd();
}

unique_ptr<Token> Lexer::next() {
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
        case '_':
            // part of identifier
            if (isAlpha(peek())) {
                return identifier();
            }
            // part of line continuation
            skipUntilLineEnd();
            if (m_char == '\n') {
                move();
            }
            continue;
        case '"':
            return string();
        case '=':
            return character(TokenKind::Assign);
        case ',':
            return character(TokenKind::Comma);
        case '.':
            if (peek(1) == '.' && peek(2) == '.') {
                return ellipsis();
            }
            return character(TokenKind::Period);
        case '(':
            return character(TokenKind::ParenOpen);
        case ')':
            return character(TokenKind::ParenClose);
        case '[':
            return character(TokenKind::BracketOpen);
        case ']':
            return character(TokenKind::BracketClose);
        case '+':
            return character(TokenKind::Plus);
        case '-':
            return character(TokenKind::Minus);
        case '*':
            return character(TokenKind::Multiply);
        case '/':
            return character(TokenKind::Divide);
        case '^':
            return character(TokenKind::Exponent);
        case '!':
            return character(TokenKind::Factorial);
        }

        return invalid(m_input);
    }

    return endOfFile(); // NOLINT
}
/**
 * If `m_hasStmt` is true then return `EndOfLine`
 * before `EndOfFile`
 */
unique_ptr<Token> Lexer::endOfFile() {
    if (m_hasStmt) {
        m_hasStmt = false;
        return std::make_unique<Token>(
            TokenKind::EndOfStmt,
            getRange(m_buffer->getBufferEnd(), m_buffer->getBufferEnd()));
    }

    return std::make_unique<Token>(
        TokenKind::EndOfFile,
        getRange(m_buffer->getBufferEnd(), m_buffer->getBufferEnd()));
}

unique_ptr<Token> Lexer::endOfStatement() {
    const auto* start = m_input;
    move();
    return std::make_unique<Token>(TokenKind::EndOfStmt, getRange(start, m_input));
}

unique_ptr<Token> Lexer::ellipsis() {
    const auto* start = m_input;
    move(3);
    return std::make_unique<Token>(TokenKind::Ellipsis, getRange(start, m_input));
}

unique_ptr<Token> Lexer::identifier() {
    const auto* start = m_input;
    size_t length = 1;
    while (move() && isAlpha(m_char)) {
        length++;
    }
    auto range = getRange(start, m_input);

    std::string uppercased;
    uppercased.reserve(length);
    std::transform(start, start + length, std::back_inserter(uppercased), llvm::toUpper);

    auto kind = Token::findKind(uppercased);
    switch (kind) {
    case TokenKind::True:
        return std::make_unique<Token>(true, range);
    case TokenKind::False:
        return std::make_unique<Token>(false, range);
    case TokenKind::Identifier:
        return std::make_unique<Token>(kind, m_context.retainCopy(uppercased), range);
    default:
        return std::make_unique<Token>(kind, range);
    }
}

unique_ptr<Token> Lexer::number() {
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
    const char* end = start + len;
    auto range = getRange(start, end);

    if (isFloatingPoint) {
        std::string number{ start, end };
        std::size_t size;
        double value = std::stod(number, &size);
        if (size == 0) {
            return invalid(start);
        }
        return std::make_unique<Token>(value, range);
    }

    uint64_t value;
    const int base10 = 10;
    if (std::from_chars(start, end, value, base10).ec != std::errc()) {
        return invalid(start);
    }
    return std::make_unique<Token>(value, range);
}

unique_ptr<Token> Lexer::string() {
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

    return std::make_unique<Token>(
        TokenKind::StringLiteral,
        m_context.retainCopy(literal),
        getRange(start, m_input));
}

unique_ptr<Token> Lexer::character(TokenKind kind) {
    const auto* start = m_input;
    move();
    return std::make_unique<Token>(kind, getRange(start, m_input));
}

unique_ptr<Token> Lexer::invalid(const char* loc) {
    return std::make_unique<Token>(TokenKind::Invalid, getRange(loc, loc));
}

void Lexer::skipUntilLineEnd() {
    while (move() && m_char != '\n') {}
}

bool Lexer::move() {
    m_char = *++m_input; // NOLINT
    handleLineEnd();
    return isValid();
}

bool Lexer::move(int steps) {
    while (steps-- > 0 && move()) {};
    return isValid();
}

void Lexer::handleLineEnd() {
    if (m_char == '\r') {
        if (peek() == '\n') {
            m_input++; // NOLINT
        }
        m_char = '\n';
    }
}

bool Lexer::isValid() const {
    return m_char != 0;
}

char Lexer::peek(int ahead) const {
    const auto* next = m_input + ahead; // NOLINT
    if (next < m_buffer->getBufferEnd()) {
        return *next;
    }
    return 0;
}
