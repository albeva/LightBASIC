//
// Created by Albert on 03/07/2020.
//
#include "Lexer/Lexer.h"
#include "Lexer/Token.h"
#include "Ast/Ast.h"
#include "Parser.h"
using namespace lbc;

Parser::Parser(llvm::SourceMgr& srcMgr, unsigned int fileId)
    : m_srcMgr(srcMgr), m_fileID(fileId) {
    m_lexer = make_unique<Lexer>(srcMgr, fileId);
    m_token = m_lexer->next();
    m_next = m_lexer->next();
}

Parser::~Parser() {}

//----------------------------------------
// Statements
//----------------------------------------

// program = stmtList .
unique_ptr<AstProgram> Parser::parse() {
    auto program = AstProgram::create();
    program->body = stmtList();
    return program;
}

// stmtList = { stmt } .
unique_ptr<AstStmtList> Parser::stmtList() {
    auto list = AstStmtList::create();
    while (isValid()) {
        while (accept(TokenKind::EndOfLine));

        if (auto s = stmt()) {
            list->stmts.emplace_back(std::move(s));
        } else {
            error("Expected statement");
        }

        expect(TokenKind::EndOfLine);
    }
    return list;
}

// stmt = decl
//      | callStmt
//      .
unique_ptr<AstStmt> Parser::stmt() {
    switch (m_token->kind()) {
    case TokenKind::Var:
        return kwVar();
    case TokenKind::Identifier:
        if (m_next && m_next->kind() == TokenKind::Assign) {
            return assign();
        }

        if (auto c = call(true)) {
            auto es = AstExprStmt::create();
            es->expr = std::move(c);
            return es;
        }
    default:
        return nullptr;
    }
}

unique_ptr<AstAssignStmt> Parser::assign() {
    return AstAssignStmt::create();
}

//----------------------------------------
// Declarations
//----------------------------------------

// var = ident "=" expr .
unique_ptr<AstVarDecl> Parser::kwVar() {
    // ident
    expect(TokenKind::Var);
    auto id = ident();
    if (!id)
        return nullptr;

    // "="
    expect(TokenKind::Assign);

    // expr
    auto e = expr();
    if (!e)
        return nullptr;

    auto var = AstVarDecl::create();
    var->id = std::move(id);
    var->expr = std::move(e);
    return var;
}

//----------------------------------------
// Expressions
//----------------------------------------

// expr = ident
//      | literal
//      | callExpr
//      .
unique_ptr<AstExpr> Parser::expr() {
    // literal
    if (m_token->isLiteral()) {
        auto lit = AstLiteralExpr::create();
        lit->literal = move();
        return lit;
    }

    if (*m_token == TokenKind::Identifier) {
        return ident();
    }

    error("Expected expression");
}

// ident = id .
unique_ptr<AstIdentExpr> Parser::ident() {
    auto id = AstIdentExpr::create();
    id->identifier = expect(TokenKind::Identifier);
    return id;
}

// stmt: call = id argList .
// expr: call = id "(" argList ")" .
unique_ptr<AstCallExpr> Parser::call(bool stmt) {
    auto call = AstCallExpr::create();
    call->ident = ident();

    bool parens;
    if (stmt) {
        parens = accept(TokenKind::ParenOpen) != nullptr;
    } else {
        expect(TokenKind::ParenOpen);
        parens = true;
    }

    while (isValid() && !match(TokenKind::ParenClose) &&  !match(TokenKind::EndOfLine)) {
        if (auto e = expr()) {
            call->arguments.emplace_back(std::move(e));
        } else {
            error("Expected expression");
        }
    }

    if (parens) {
        expect(TokenKind::ParenClose);
    }

    return call;
}

//----------------------------------------
// Helpers
//----------------------------------------

bool Parser::isValid() const {
    assert(m_token);
    return *m_token != TokenKind::EndOfFile;
}

bool Parser::match(TokenKind kind) const {
    return *m_token == kind;
}

unique_ptr<Token> Parser::accept(TokenKind kind) {
    if (match(kind)) {
        return move();
    }
    return nullptr;
}

unique_ptr<Token> Parser::expect(TokenKind kind) {
    if (!match(kind)) {
        error(llvm::Twine("Expected '")
            .concat(view_to_stringRef(Token::description(kind)))
            .concat("' got '")
            .concat(view_to_stringRef(m_token->description()))
            .concat("'")
        );
    }
    return move();
}

unique_ptr<Token> Parser::move() {
    auto current = std::move(m_token);
    m_token = std::move(m_next);
    m_next = m_lexer->next();
    return current;
}

[[noreturn]]
void Parser::error(const llvm::Twine& message) {
    m_srcMgr.PrintMessage(
        m_token->loc(),
        llvm::SourceMgr::DK_Error,
        message,
        m_token->range()
    );
    std::exit(EXIT_FAILURE);
}