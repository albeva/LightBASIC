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

/**
 * Program = stmtList .
 */
unique_ptr<AstProgram> Parser::parse() {
    auto program = AstProgram::create();
    program->body = stmtList();
    return program;
}

//----------------------------------------
// Statements
//----------------------------------------

/**
 * stmtList = { statement } .
 */
unique_ptr<AstStmtList> Parser::stmtList() {
    auto list = AstStmtList::create();
    while (isValid()) {
        list->stmts.emplace_back(statement());
        expect(TokenKind::EndOfStmt);
    }
    return list;
}

/**
 * statement = VAR
 *           | assignStmt
 *           | callStmt
 *           .
 */
unique_ptr<AstStmt> Parser::statement() {
    switch (m_token->kind()) {
    case TokenKind::BracketOpen:
    case TokenKind::Var:
        return declaration();
    case TokenKind::Identifier:
        if (m_next && m_next->kind() == TokenKind::Assign) {
            return assignStmt();
        }
        return callStmt();
    default:
        error("Expected statement");
    }
}

/**
 * assignStmt = ident '=' expression .
 */
unique_ptr<AstAssignStmt> Parser::assignStmt() {
    auto ident = identifier();
    expect(TokenKind::Assign);
    auto expr = expression();

    auto assign = AstAssignStmt::create();
    assign->ident = std::move(ident);
    assign->expr = std::move(expr);
    return assign;
}

/**
 * CallStmt = id
 *          ( '(' ArgumentList ')'
 *          | ArgumentList
 *          )
 *          .
 */
unique_ptr<AstExprStmt> Parser::callStmt() {
    auto call = AstCallExpr::create();
    call->ident = identifier();

    bool parens = accept(TokenKind::ParenOpen) != nullptr;

    call->arguments = expressionList();

    if (parens) {
        expect(TokenKind::ParenClose);
    }

    auto stmt = AstExprStmt::create();
    stmt->expr = std::move(call);
    return stmt;
}

//----------------------------------------
// Declarations
//----------------------------------------

/**
 * Decl = [ '[' attributeList '] ]
 *      ( VAR
 *      )
 *      .
 */
unique_ptr<AstDecl> Parser::declaration() {
    unique_ptr<AstAttributeList> attribs = nullptr;
    if (*m_token == TokenKind::BracketOpen) {
        attribs = attributeList();
    }

    auto decl = kwVar();
    decl->attribs = std::move(attribs);
    return decl;
}

/**
 *  attributeList = '[' Attribute { ','  Attribute } ']' .
 */
unique_ptr<AstAttributeList> Parser::attributeList() {
    auto list = AstAttributeList::create();

    expect(TokenKind::BracketOpen);
    do {
        list->attribs.emplace_back(attribute());
    } while (accept(TokenKind::Comma));
    expect(TokenKind::BracketClose);

    return list;
}

/**
 * attribute = "id" [AttributeArgList] .
 */
unique_ptr<AstAttribute> Parser::attribute() {
    auto attrib = AstAttribute::create();
    attrib->ident = identifier();

    if (*m_token == TokenKind::Assign || *m_token == TokenKind::ParenOpen) {
        attrib->arguments = attributeArgumentList();
    }

    return attrib;
}

/**
 * AttributeArgList = '=' AttributeArgument
 *                  | '(' [ AttributeArgument { ',' AttributeArgument } ] ')'
 *                  .
 * AttributeArgument = StringLiteral
 *                   | NumberLiteral
 *                   | BooleanLiteral
 *                   | NullLiteral
 *                   .
 */
vector<unique_ptr<AstLiteralExpr>> Parser::attributeArgumentList() {
    vector<unique_ptr<AstLiteralExpr>> args;
    if (accept(TokenKind::Assign)) {
        args.emplace_back(literalExpr());
    } else if (accept(TokenKind::ParenOpen)) {
        while (isValid() && *m_token != TokenKind::ParenClose) {
            args.emplace_back(literalExpr());
            if (!accept(TokenKind::Comma))
                break;
        }
        expect(TokenKind::ParenClose);
    }
    return args;
}

/**
 * var = identifier "=" expression .
 */
unique_ptr<AstVarDecl> Parser::kwVar() {
    // identifier
    expect(TokenKind::Var);
    auto id = identifier();
    if (!id)
        return nullptr;

    // "="
    expect(TokenKind::Assign);

    // expression
    auto expr = expression();
    if (!expr)
        return nullptr;

    auto var = AstVarDecl::create();
    var->ident = std::move(id);
    var->expr = std::move(expr);
    return var;
}

//----------------------------------------
// Expressions
//----------------------------------------

/**
 * expression = literal
 *            | identifier
 *            | callExpr
 *            .
 */
unique_ptr<AstExpr> Parser::expression() {
    // literal
    if (m_token->isLiteral()) {
        return literalExpr();
    }

    if (*m_token == TokenKind::Identifier) {
        if (m_next && *m_next == TokenKind::ParenOpen) {
            return callExpr();
        }
        return identifier();
    }

    error("Expected expression");
}

/**
 * ident = identifier .
 */
unique_ptr<AstIdentExpr> Parser::identifier() {
    auto id = AstIdentExpr::create();
    id->token = expect(TokenKind::Identifier);
    return id;
}

/**
 * callExpr = identifier "(" argList ")" .
 */
unique_ptr<AstCallExpr> Parser::callExpr() {
    auto call = AstCallExpr::create();
    call->ident = identifier();

    expect(TokenKind::ParenOpen);
    call->arguments = expressionList();
    expect(TokenKind::ParenClose);

    return call;
}

unique_ptr<AstLiteralExpr> Parser::literalExpr() {
    if (m_token->isLiteral()) {
        auto lit = AstLiteralExpr::create();
        lit->token = move();
        return lit;
    } else {
        error("Expected literal");
    }
}

/**
 * Parse comma separated list of expressionds
 */
vector<unique_ptr<AstExpr>> Parser::expressionList() {
    vector<unique_ptr<AstExpr>> exprs;

    while (isValid() && !match(TokenKind::ParenClose) && !match(TokenKind::EndOfStmt)) {
        if (auto e = expression()) {
            exprs.emplace_back(std::move(e));
        } else {
            error("Expected expression");
        }
    }

    return exprs;
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
