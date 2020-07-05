//
// Created by Albert on 03/07/2020.
//
#include "Lexer.h"

using namespace lbc;

using llvm::isAlpha;

static inline bool isWhiteSpace(char ch) {
    return ch == ' ' || ch == '\t';
}

static inline llvm::SMLoc getLoc(const char *ptr) {
    return llvm::SMLoc::getFromPointer(ptr);
}

Lexer::Lexer(llvm::SourceMgr& srcMgr, unsigned fileID)
    : m_srcMgr{srcMgr},
      m_fileID{fileID},
      m_buffer{srcMgr.getMemoryBuffer(fileID)} {
    assert(m_buffer != nullptr);
    m_input = m_buffer->getBufferStart();
    m_char = *m_input;
}

unique_ptr<Token> Lexer::next() {
    while (isValid()) {
        if (isWhiteSpace(m_char)) {
            move();
            continue;
        }

        if (isAlpha(m_char))
            return identifier();

        switch (m_char) {
        case '\n':
            return endOfLine();
        case '\'':
            return singleLineComment();
        case '_':
            if (isAlpha(peek())) {
                return identifier();
            }
            break;
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

    return endOfFile();
}

unique_ptr<Token> Lexer::endOfFile() {
    return Token::create(
        TokenKind::EndOfFile,
        Token::description(TokenKind::EndOfFile),
        getLoc(m_buffer->getBufferEnd())
    );
}

unique_ptr<Token> Lexer::endOfLine() {
    auto loc = getLoc(m_input);
    move();
    return Token::create(TokenKind::EndOfLine, Token::description(TokenKind::EndOfLine), loc);
}

unique_ptr<Token> Lexer::identifier() {
    const auto *start = m_input;
    do { move(); } while (isAlpha(m_char));
    auto length = static_cast<string_view::size_type>(m_input - start);
    return Token::create({start, length}, getLoc(start));
}

unique_ptr<Token> Lexer::string() {
    const auto *start = m_input;
    do {
        move();
        if (m_char == '"') {
            auto length = static_cast<string_view::size_type>(m_input - start - 1);
            move();
            return Token::create(TokenKind::StringLiteral, {start + 1, length}, getLoc(start));
        }
    } while (m_char > 31);

    return invalid(start);
}

unique_ptr<Token> Lexer::character(TokenKind kind) {
    const auto *start = m_input;
    move();
    return Token::create(kind, {start, 1}, getLoc(start));
}

unique_ptr<Token> Lexer::invalid(const char *loc) {
    return Token::create(TokenKind::Invalid, {}, getLoc(loc));
}

unique_ptr<Token> Lexer::singleLineComment() {
    do {
        move();
        if (m_char == '\n')
            return endOfLine();
    } while (isValid());

    return endOfFile();
}

void Lexer::move() {
    m_char = *++m_input;
}

bool Lexer::isValid() const {
    return m_char != 0;
}

char Lexer::peek(int ahead) const {
    const auto *next = m_input + ahead;
    if (next < m_buffer->getBufferEnd()) {
        return *next;
    } else {
        return 0;
    }
}
