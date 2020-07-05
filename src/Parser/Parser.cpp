//
// Created by Albert on 03/07/2020.
//
#include "Parser.h"
#include "Lexer/Lexer.h"
#include "Lexer/Token.h"
#include "Ast.h"

using namespace lbc;

Parser::Parser(llvm::SourceMgr& srcMgr, unsigned int fileId)
    : m_srcMgr(srcMgr), m_fileID(fileId) {
    m_lexer = make_unique<Lexer>(srcMgr, fileId);
    m_token = m_lexer->next();
    m_next = m_lexer->next();
}

Parser::~Parser() {}

unique_ptr<AstStmtList> Parser::parse() {
    auto list = make_unique<AstStmtList>();

    while (isValid()) {
        if (*m_token == TokenKind::EndOfLine) {
            move();
            continue;
        }
        move();
    }

    return list;
}

bool Parser::isValid() const {
    assert(m_token);
    return *m_token != TokenKind::EndOfFile;
}

bool Parser::match(TokenKind kind) const {
    return *m_token == kind;
}

bool Parser::accept(TokenKind kind) {
    if (match(kind)) {
        move();
        return true;
    }
    return false;
}

void Parser::expect(TokenKind kind) {
    if (!match(kind)) {
        auto message = llvm::Twine("Expected '") // NOLINT
            .concat(view_to_stringRef(Token::description(kind)))
            .concat("' got '")
            .concat(view_to_stringRef(m_token->description()))
            .concat("'");

        m_srcMgr.PrintMessage(
            m_token->loc(),
            llvm::SourceMgr::DK_Error,
            message,
            m_token->range()
        );

        std::exit(EXIT_FAILURE);
    }
}

void Parser::move() {
    m_token = std::move(m_next);
    m_next = m_lexer->next();
}
