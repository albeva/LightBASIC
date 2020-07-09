//
// Created by Albert on 03/07/2020.
//
#include "Lexer.h"
#include "Token.h"
using namespace lbc;

using llvm::isAlpha;

static inline bool isWhiteSpace(char ch) {
    return ch == ' ' || ch == '\t';
}

static inline llvm::SMLoc getLoc(const char* ptr) {
    return llvm::SMLoc::getFromPointer(ptr);
}

Lexer::Lexer(llvm::SourceMgr& srcMgr, unsigned fileID)
  : m_srcMgr{ srcMgr },
    m_fileID{ fileID },
    m_buffer{ srcMgr.getMemoryBuffer(fileID) },
    m_hasStmt{ false } {
    m_input = m_buffer->getBufferStart();
    m_char = *m_input;
}

unique_ptr<Token> Lexer::next() {
    while (isValid()) {
        // skip spaces
        if (isWhiteSpace(m_char)) {
            move();
            continue;
        }

        // new line, emit statement if there is one
        if (m_char == '\n') {
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
    return Token::create(TokenKind::EndOfStmt, Token::description(TokenKind::EndOfStmt), loc);
}

unique_ptr<Token> Lexer::identifier() {
    const auto* start = m_input;
    do {
        move();
    } while (isAlpha(m_char));
    auto length = static_cast<string_view::size_type>(m_input - start);
    return Token::create({ start, length }, getLoc(start));
}

unique_ptr<Token> Lexer::string() {
    constexpr char visibleFrom = 32;

    const auto* start = m_input;
    do {
        move();
        if (m_char == '"') {
            auto length = static_cast<string_view::size_type>(m_input - start - 1);
            move();
            return Token::create(TokenKind::StringLiteral, { start + 1, length }, getLoc(start)); // NOLINT
        }
    } while (m_char >= visibleFrom);

    return invalid(start);
}

unique_ptr<Token> Lexer::character(TokenKind kind) {
    const auto* start = m_input;
    move();
    return Token::create(kind, { start, 1 }, getLoc(start));
}

unique_ptr<Token> Lexer::invalid(const char* loc) {
    return Token::create(TokenKind::Invalid, {}, getLoc(loc));
}

void Lexer::skipUntilLineEnd() {
    do {
        move();
    } while (isValid() && m_char != '\n');
}

void Lexer::move() {
    m_char = *++m_input; // NOLINT
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
