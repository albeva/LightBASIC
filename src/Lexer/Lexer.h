//
// Created by Albert on 03/07/2020.
//
#pragma once
#include "pch.h"
#include "Token.h"

namespace lbc {

class Lexer final : noncopyable {
public:
    Lexer(llvm::SourceMgr& srcMgr, unsigned fileID);

    unique_ptr<Token> next();

private:

    unique_ptr<Token> identifier();
    unique_ptr<Token> string();
    unique_ptr<Token> character(TokenKind kind);
    unique_ptr<Token> endOfLine();
    unique_ptr<Token> endOfFile();
    unique_ptr<Token> singleLineComment();
    unique_ptr<Token> invalid(const char *loc);

    void move();
    bool isValid() const;
    char peek(int ahead = 1) const;

    llvm::SourceMgr& m_srcMgr;
    unsigned m_fileID;
    const llvm::MemoryBuffer *m_buffer;
    const char *m_input;
    char m_char;
};

} // namespace lbc
