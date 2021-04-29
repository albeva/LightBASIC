//
// Created by Albert Varaksin on 03/07/2020.
//
#include "Parser.h"
#include "Ast/Ast.h"
#include "Driver/Context.h"
#include "Lexer/Lexer.h"
#include "Lexer/Token.h"
using namespace lbc;

Parser::Parser(Context& context, unsigned int fileId, bool isMain)
: m_context{ context },
  m_fileId{ fileId },
  m_isMain{ isMain },
  m_scope{ Scope::Root } {
    m_lexer = make_unique<Lexer>(m_context, fileId);
    m_token = m_lexer->next();
    m_next = m_lexer->next();
}

/**
 * Program = stmtList .
 */
unique_ptr<AstModule> Parser::parse() {
    auto module = AstModule::create();
    module->fileId = m_fileId;
    module->hasImplicitMain = m_isMain;
    module->stmtList = stmtList();
    expect(TokenKind::EndOfFile);
    return module;
}

//----------------------------------------
// Statements
//----------------------------------------

/**
 * stmtList = { statement } .
 */
unique_ptr<AstStmtList> Parser::stmtList() {
    auto list = AstStmtList::create();
    while (isValid() && !match(TokenKind::End)) {
        list->stmts.emplace_back(statement());
        expect(TokenKind::EndOfStmt);
    }
    return list;
}

/**
 * statement =
 *           ( [ attributeList ]
 *             ( VAR
 *             | DECLARE
 *             | FUNCTION
 *             | SUB
 *             )
 *           )
 *           | assignStmt
 *           | callStmt
 *           .
 */
unique_ptr<AstStmt> Parser::statement() {
    unique_ptr<AstAttributeList> attribs;
    if (m_token->kind() == TokenKind::BracketOpen) {
        attribs = attributeList();
    }

    switch (m_token->kind()) {
    case TokenKind::Var:
        return kwVar(std::move(attribs));
    case TokenKind::Declare:
        return kwDeclare(std::move(attribs));
    case TokenKind::Function:
    case TokenKind::Sub:
        return kwFunction(std::move(attribs));
    default:
        break;
    }

    if (attribs) {
        error("Expected SUB, FUNCTION, DECLARE or VAR got '"_t
            + m_token->description()
            + "'");
    }

    if (!m_isMain && m_scope == Scope::Root) {
        error("expressions are not allowed at the top level");
    }

    switch (m_token->kind()) {
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
 * assignStmt = identExpr '=' expression .
 */
unique_ptr<AstAssignStmt> Parser::assignStmt() {
    auto ident = identifier();
    expect(TokenKind::Assign);
    auto expr = expression();

    auto assign = AstAssignStmt::create();
    assign->identExpr = std::move(ident);
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
    call->identExpr = identifier();

    bool parens = accept(TokenKind::ParenOpen) != nullptr;

    call->argExprs = expressionList();

    if (parens) {
        expect(TokenKind::ParenClose);
    }

    auto stmt = AstExprStmt::create();
    stmt->expr = std::move(call);
    return stmt;
}

/**
 *  funcStmt = funcSignature <EoS>
 *             stmtList
 *             "END" ("FUNCTION" | "SUB")
 */
unique_ptr<AstFuncStmt> Parser::kwFunction(unique_ptr<AstAttributeList> attribs) {
    if (m_scope != Scope::Root) {
        error("Nested SUBs/FUNCTIONs not allowed");
    }

    auto func = AstFuncStmt::create();
    func->decl = funcSignature(std::move(attribs));
    expect(TokenKind::EndOfStmt);

    RESTORE_ON_EXIT(m_scope)
    m_scope = Scope::Function;

    func->stmtList = stmtList();

    expect(TokenKind::End);
    if (func->decl->retTypeExpr == nullptr) {
        expect(TokenKind::Sub);
    } else {
        expect(TokenKind::Function);
    }

    return func;
}

//----------------------------------------
// Attributes
//----------------------------------------

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
    attrib->identExpr = identifier();

    if (*m_token == TokenKind::Assign || *m_token == TokenKind::ParenOpen) {
        attrib->argExprs = attributeArgumentList();
    }

    return attrib;
}

/**
 * AttributeArgList = '=' literal
 *                  | '(' [ literalExpr { ',' literal } ] ')'
 *                  .
 */
std::vector<unique_ptr<AstLiteralExpr>> Parser::attributeArgumentList() {
    std::vector<unique_ptr<AstLiteralExpr>> args;
    if (accept(TokenKind::Assign)) {
        args.emplace_back(literal());
    } else if (accept(TokenKind::ParenOpen)) {
        while (isValid() && *m_token != TokenKind::ParenClose) {
            args.emplace_back(literal());
            if (!accept(TokenKind::Comma)) {
                break;
            }
        }
        expect(TokenKind::ParenClose);
    }
    return args;
}

//----------------------------------------
// Declarations
//----------------------------------------

/**
 * var = identifier
 *     ( "=" expression
 *     | "AS" TypeExpr [ "=" expression ]
 *     ) .
 */
unique_ptr<AstVarDecl> Parser::kwVar(unique_ptr<AstAttributeList> attribs) {
    expect(TokenKind::Var);

    auto id = expect(TokenKind::Identifier);

    unique_ptr<AstTypeExpr> type;
    unique_ptr<AstExpr> expr;

    if (accept(TokenKind::As)) {
        type = typeExpr();
        if (accept(TokenKind::Assign)) {
            expr = expression();
        }
    } else {
        expect(TokenKind::Assign);
        expr = expression();
    }

    auto var = AstVarDecl::create();
    var->attributes = std::move(attribs);
    var->token = std::move(id);
    var->typeExpr = std::move(type);
    var->expr = std::move(expr);
    return var;
}

/**
 * kwDeclare = "DECLARE" funcSignature .
 */
unique_ptr<AstFuncDecl> Parser::kwDeclare(unique_ptr<AstAttributeList> attribs) {
    if (m_scope != Scope::Root) {
        error("Nested declarations not allowed");
    }
    expect(TokenKind::Declare);
    return funcSignature(std::move(attribs));
}

/**
 * funcSignature = ( "FUNCTION" id [ "(" funcParams ")" ] "AS" TypeExpr
 *                 | "SUB" id [ "(" funcParams ")" ]
 *                 ) .
 */
unique_ptr<AstFuncDecl> Parser::funcSignature(unique_ptr<AstAttributeList> attribs) {
    auto func = AstFuncDecl::create();
    func->attributes = std::move(attribs);

    bool isFunc = accept(TokenKind::Function) != nullptr;
    if (!isFunc) {
        expect(TokenKind::Sub);
    }

    func->token = expect(TokenKind::Identifier);

    if (accept(TokenKind::ParenOpen)) {
        bool isVariadic = false;
        func->paramDecls = funcParams(isVariadic);
        func->variadic = isVariadic;
        expect(TokenKind::ParenClose);
    }

    if (isFunc) {
        expect(TokenKind::As);
        func->retTypeExpr = typeExpr();
    }

    return func;
}

/**
 *  funcParams = <empty>
 *             | Param { "," Param } [ "," "..." ]
 *             | "..."
 *             .
 *  Param = id "AS" TypeExpr .
 */
std::vector<unique_ptr<AstFuncParamDecl>> Parser::funcParams(bool& isVariadic) {
    std::vector<unique_ptr<AstFuncParamDecl>> params;

    while (isValid() && *m_token != TokenKind::ParenClose) {
        if (accept(TokenKind::Ellipsis)) {
            isVariadic = true;
            if (match(TokenKind::Comma)) {
                error("Variadic parameter must be last in function declaration");
            }
            break;
        }

        auto id = expect(TokenKind::Identifier);
        expect(TokenKind::As);
        auto type = typeExpr();

        auto param = AstFuncParamDecl::create();
        param->token = std::move(id);
        param->typeExpr = std::move(type);
        params.push_back(std::move(param));

        if (!accept(TokenKind::Comma)) {
            break;
        }
    }
    return params;
}

//----------------------------------------
// Types
//----------------------------------------

unique_ptr<AstTypeExpr> Parser::typeExpr() {
#define TYPE_KEYWORD(id, ...) case TokenKind::id:
    switch (m_token->kind()) {
        ALL_TYPES(TYPE_KEYWORD)
        {
            auto type = AstTypeExpr::create();
            type->token = move();
            return type;
        }
    default:
        error("Expected typeExpr");
    }
#undef TYPE_KEYWORD
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
        return literal();
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
 * identExpr = identifier .
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
    call->identExpr = identifier();

    expect(TokenKind::ParenOpen);
    call->argExprs = expressionList();
    expect(TokenKind::ParenClose);

    return call;
}

unique_ptr<AstLiteralExpr> Parser::literal() {
    if (!m_token->isLiteral()) {
        error("Expected literal");
    }
    auto lit = AstLiteralExpr::create();
    lit->token = move();
    return lit;
}

/**
 * Parse comma separated list of expressionds
 */
std::vector<unique_ptr<AstExpr>> Parser::expressionList() {
    std::vector<unique_ptr<AstExpr>> exprs;

    while (isValid() && !match(TokenKind::ParenClose) && !match(TokenKind::EndOfStmt)) {
        exprs.emplace_back(expression());
        if (!accept(TokenKind::Comma)) {
            break;
        }
    }

    return exprs;
}

//----------------------------------------
// Helpers
//----------------------------------------

bool Parser::isValid() const {
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
        error("Expected '"_t
            + Token::description(kind)
            + "' got '"
            + m_token->description()
            + "'");
    }
    return move();
}

unique_ptr<Token> Parser::move() {
    auto current = std::move(m_token);
    m_token = std::move(m_next);
    m_next = m_lexer->next();
    return current;
}

[[noreturn]] void Parser::error(const Twine& message) {
    string output;
    llvm::raw_string_ostream stream{ output };

    m_context.getSourceMrg().PrintMessage(
        stream,
        m_token->loc(),
        llvm::SourceMgr::DK_Error,
        message,
        m_token->range());

    fatalError(output, false);
}
