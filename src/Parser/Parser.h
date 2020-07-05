//
// Created by Albert on 03/07/2020.
//
#pragma once

#include "pch.h"
#include "Ast.def.h"

namespace lbc {

class Lexer;
class Token;
enum class TokenKind;
AST_FORWARD_DECLARE()

class Parser final : noncopyable {
public:
    Parser(llvm::SourceMgr& srcMgr, unsigned int fileId);

    ~Parser();

    unique_ptr<AstStmtList> parse();

private:

    // is current token a valid input?
    bool isValid() const;

    // match current character
    bool match(TokenKind kind) const;

    // Accept character and move to the next
    bool accept(TokenKind kind);

    // expect a character. Throw an error if doesn't match
    void expect(TokenKind kind);

    // Move to the next one
    void move();

    llvm::SourceMgr& m_srcMgr;
    unsigned m_fileID;
    std::unique_ptr<Lexer> m_lexer;
    std::unique_ptr<Token> m_token;
    std::unique_ptr<Token> m_next;
};

} // namespace lbc