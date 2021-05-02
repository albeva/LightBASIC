//
// Created by Albert Varaksin on 03/07/2020.
//
#include "Lexer.h"
#include "Driver/Context.h"
#include "Token.h"
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

inline llvm::SMLoc getLoc(const char* ptr) {
    return llvm::SMLoc::getFromPointer(ptr);
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
        if (isDigit(m_char)) {
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
        return Token::create(
            TokenKind::EndOfStmt,
            Token::description(TokenKind::EndOfStmt),
            getLoc(m_buffer->getBufferEnd()));
    }

    return Token::create(
        TokenKind::EndOfFile,
        Token::description(TokenKind::EndOfFile),
        getLoc(m_buffer->getBufferEnd()));
}

unique_ptr<Token> Lexer::endOfStatement() {
    auto loc = getLoc(m_input);
    move();
    return Token::create(TokenKind::EndOfStmt, loc);
}

unique_ptr<Token> Lexer::ellipsis() {
    auto loc = getLoc(m_input);
    move(3);
    return Token::create(TokenKind::Ellipsis, loc);
}

unique_ptr<Token> Lexer::identifier() {
    const auto* start = m_input;
    size_t length = 1;
    while (move() && isAlpha(m_char)) {
        length++;
    }
    return Token::create({ start, length }, getLoc(start));
}

unique_ptr<Token> Lexer::number() {
    const auto* start = m_input;
    size_t len = 1;
    while (move() && isDigit(m_char)) {
        len++;
    }
    return Token::create(TokenKind::NumberLiteral, { start, len }, getLoc(start));
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

    return Token::create(TokenKind::StringLiteral, literal, getLoc(start)); // NOLINT
}

unique_ptr<Token> Lexer::character(TokenKind kind) {
    const auto* start = m_input;
    move();
    return Token::create(kind, getLoc(start));
}

unique_ptr<Token> Lexer::invalid(const char* loc) {
    return Token::create(TokenKind::Invalid, {}, getLoc(loc));
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
