//
// Created by Albert Varaksin on 03/07/2020.
//
#include "Parser.h"
#include "Ast/Ast.h"
#include "Driver/Context.h"
#include "Lexer/Lexer.h"
#include "Lexer/Token.h"
#include "Type/Type.h"
#include <limits>
using namespace lbc;

Parser::Parser(Context& context, unsigned int fileId, bool isMain) noexcept
: m_context{ context },
  m_fileId{ fileId },
  m_isMain{ isMain },
  m_scope{ Scope::Root } {
    m_lexer = make_unique<Lexer>(m_context, fileId);
    m_token = m_lexer->next();
    m_next = m_lexer->next();
    m_endLoc = m_token->range().End; // NOLINT
}

/**
 * Module
 *   = StmtList
 *   .
 */
unique_ptr<AstModule> Parser::parse() noexcept {
    auto stmts = stmtList();

    auto module = AstModule::create(stmts->getRange());
    module->fileId = m_fileId;
    module->hasImplicitMain = m_isMain;
    module->stmtList = std::move(stmts);
    expect(TokenKind::EndOfFile);
    return module;
}

//----------------------------------------
// Statements
//----------------------------------------

/**
 * StmtList
 *   = { Statement }
 *   .
 */
unique_ptr<AstStmtList> Parser::stmtList() noexcept {
    constexpr auto isTerminator = [](const unique_ptr<Token>& token) noexcept {
        switch (token->kind()) {
        case TokenKind::End:
        case TokenKind::Else:
        case TokenKind::Next:
            return true;
        default:
            return false;
        }
    };

    auto start = m_token->range().Start;
    std::vector<unique_ptr<AstStmt>> stms;
    while (isValid() && !isTerminator(m_token)) {
        stms.emplace_back(statement());
        expect(TokenKind::EndOfStmt);
    }
    auto list = AstStmtList::create({ start, m_endLoc });
    list->stmts = std::move(stms);
    return list;
}

/**
 * Statement
 *   = Declaration
 *   | Assignment
 *   | CallStmt
 *   | IfStmt
 *   | ForStmt
 *   | RETURN
 *   .
 */
unique_ptr<AstStmt> Parser::statement() noexcept {
    if (auto decl = declaration()) {
        return decl;
    }

    if (!m_isMain && m_scope == Scope::Root) {
        error("expressions are not allowed at the top level");
    }

    switch (m_token->kind()) {
    case TokenKind::Identifier:
        if (m_next && m_next->kind() == TokenKind::Assign) {
            return assignment();
        }
        return callStmt();
    case TokenKind::Return:
        return kwReturn();
    case TokenKind::If:
        return kwIf();
    case TokenKind::For:
        return kwFor();
    default:
        error("Expected statement");
    }
}

/**
 * Declaration
 *   = [ AttributeList ]
 *   ( VAR
 *   | DECLARE
 *   | FUNCTION
 *   | SUB
 *   )
 *   .
 */
unique_ptr<AstStmt> Parser::declaration() noexcept {
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
    return nullptr;
}

//----------------------------------------
// Attributes
//----------------------------------------

/**
 *  AttributeList = '[' Attribute { ','  Attribute } ']' .
 */
unique_ptr<AstAttributeList> Parser::attributeList() noexcept {
    auto start = m_token->range().Start;

    std::vector<unique_ptr<AstAttribute>> attribs;
    expect(TokenKind::BracketOpen);
    do {
        attribs.emplace_back(attribute());
    } while (accept(TokenKind::Comma));
    expect(TokenKind::BracketClose);

    auto list = AstAttributeList::create({ start, m_endLoc });
    list->attribs = std::move(attribs);
    return list;
}

/**
 * Attribute
 *   = IdentExpr [ AttributeArgList ]
 *   .
 */
unique_ptr<AstAttribute> Parser::attribute() noexcept {
    auto start = m_token->range().Start;

    auto id = identifier();
    std::vector<unique_ptr<AstLiteralExpr>> args;
    if (*m_token == TokenKind::Assign || *m_token == TokenKind::ParenOpen) {
        args = attributeArgList();
    }

    auto attrib = AstAttribute::create({ start, m_endLoc });
    attrib->identExpr = std::move(id);
    attrib->argExprs = std::move(args);
    return attrib;
}

/**
 * AttributeArgList
 *   = "=" Literal
 *   | "(" [ Literal { "," Literal } ] ")"
 *   .
 */
std::vector<unique_ptr<AstLiteralExpr>> Parser::attributeArgList() noexcept {
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
// VAR
//----------------------------------------

/**
 * VAR
 *   = "VAR" identifier
 *   ( "=" Expression
 *   | "AS" TypeExpr [ "=" Expression ]
 *   )
 *   .
 */
unique_ptr<AstVarDecl> Parser::kwVar(unique_ptr<AstAttributeList> attribs) noexcept {
    auto start = attribs == nullptr ? m_token->range().Start : attribs->getRange().Start;

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

    auto var = AstVarDecl::create({ start, m_endLoc });
    var->attributes = std::move(attribs);
    var->id = std::get<StringRef>(id->getValue());
    var->typeExpr = std::move(type);
    var->expr = std::move(expr);
    return var;
}

//----------------------------------------
// DECLARE
//----------------------------------------

/**
 * DECLARE
 *   = "DECLARE" FuncSignature
 *   .
 */
unique_ptr<AstFuncDecl> Parser::kwDeclare(unique_ptr<AstAttributeList> attribs) noexcept {
    if (m_scope != Scope::Root) {
        error("Nested declarations not allowed");
    }
    auto start = attribs == nullptr ? m_token->range().Start : attribs->getRange().Start;
    expect(TokenKind::Declare);
    return funcSignature(start, std::move(attribs));
}

/**
 * FuncSignature
 *     = "FUNCTION" id [ "(" [ FuncParamList ] ")" ] "AS" TypeExpr
 *     | "SUB" id [ "(" FuncParamList ")" ]
 *     .
 */
unique_ptr<AstFuncDecl> Parser::funcSignature(llvm::SMLoc start, unique_ptr<AstAttributeList> attribs) noexcept {
    bool isFunc = accept(TokenKind::Function) != nullptr;
    if (!isFunc) {
        expect(TokenKind::Sub);
    }

    auto id = expect(TokenKind::Identifier);

    bool isVariadic = false;
    std::vector<unique_ptr<AstFuncParamDecl>> params;
    if (accept(TokenKind::ParenOpen)) {
        params = funcParamList(isVariadic);
        expect(TokenKind::ParenClose);
    }

    unique_ptr<AstTypeExpr> ret;
    if (isFunc) {
        expect(TokenKind::As);
        ret = typeExpr();
    }

    auto func = AstFuncDecl::create({ start, m_endLoc });
    func->id = std::get<StringRef>(id->getValue());
    func->attributes = std::move(attribs);
    func->variadic = isVariadic;
    func->paramDecls = std::move(params);
    func->retTypeExpr = std::move(ret);
    return func;
}

/**
 * FuncParamList
 *   = FuncParam { "," FuncParam } [ "," "..." ]
 *   | "..."
 *   .
 */
std::vector<unique_ptr<AstFuncParamDecl>> Parser::funcParamList(bool& isVariadic) noexcept {
    std::vector<unique_ptr<AstFuncParamDecl>> params;
    while (isValid() && *m_token != TokenKind::ParenClose) {
        if (accept(TokenKind::Ellipsis)) {
            isVariadic = true;
            if (match(TokenKind::Comma)) {
                error("Variadic parameter must be last in function declaration");
            }
            break;
        }
        params.push_back(funcParam());
        if (!accept(TokenKind::Comma)) {
            break;
        }
    }
    return params;
}

/**
 * FuncParam
 *  = id "AS" TypeExpr
 *  .
 */
unique_ptr<AstFuncParamDecl> Parser::funcParam() noexcept {
    auto start = m_token->range().Start;

    auto id = expect(TokenKind::Identifier);
    expect(TokenKind::As);
    auto type = typeExpr();

    auto param = AstFuncParamDecl::create({ start, m_endLoc });
    param->id = std::get<StringRef>(id->getValue());
    param->typeExpr = std::move(type);
    return param;
}

//----------------------------------------
// Assignment
//----------------------------------------

/**
 * Assignment
 *   = identExpr '=' expression .
 */
unique_ptr<AstAssignStmt> Parser::assignment() noexcept {
    auto start = m_token->range().Start;
    auto ident = identifier();
    expect(TokenKind::Assign);
    auto expr = expression();

    auto assign = AstAssignStmt::create({ start, m_endLoc });
    assign->identExpr = std::move(ident);
    assign->expr = std::move(expr);
    return assign;
}

//----------------------------------------
// Call
//----------------------------------------

/**
 * CallStmt = id
 *          ( '(' ArgumentList ')'
 *          | ArgumentList
 *          )
 *          .
 */
unique_ptr<AstExprStmt> Parser::callStmt() noexcept {
    auto start = m_token->range().Start;
    auto id = identifier();
    bool parens = accept(TokenKind::ParenOpen) != nullptr;
    auto args = expressionList();
    if (parens) {
        expect(TokenKind::ParenClose);
    }

    auto call = AstCallExpr::create({ start, m_endLoc });
    call->identExpr = std::move(id);
    call->argExprs = std::move(args);

    auto stmt = AstExprStmt::create(call->getRange());
    stmt->expr = std::move(call);
    return stmt;
}

/**
 *  FUNCTION = funcSignature <EoS>
 *             stmtList
 *             "END" ("FUNCTION" | "SUB")
 */
unique_ptr<AstFuncStmt> Parser::kwFunction(unique_ptr<AstAttributeList> attribs) noexcept {
    if (m_scope != Scope::Root) {
        error("Nested SUBs/FUNCTIONs not allowed");
    }

    auto start = attribs == nullptr ? m_token->range().Start : attribs->getRange().Start;
    auto decl = funcSignature(start, std::move(attribs));
    decl->hasImpl = true;
    expect(TokenKind::EndOfStmt);

    RESTORE_ON_EXIT(m_scope);
    m_scope = Scope::Function;

    auto stmts = stmtList();

    expect(TokenKind::End);
    if (decl->retTypeExpr) {
        expect(TokenKind::Function);
    } else {
        expect(TokenKind::Sub);
    }

    auto func = AstFuncStmt::create({ start, m_endLoc });
    func->decl = std::move(decl);
    func->stmtList = std::move(stmts);
    return func;
}

/**
 * RETURN = "RETURN" [ expression ] .
 */
unique_ptr<AstStmt> Parser::kwReturn() noexcept {
    if (m_scope == Scope::Root) {
        error("Unexpected RETURN outside SUB / FUNCTION body");
    }
    auto start = m_token->range().Start;
    expect(TokenKind::Return);

    unique_ptr<AstExpr> expr;
    if (!match(TokenKind::EndOfStmt)) {
        expr = expression();
    }

    auto ret = AstReturnStmt::create({ start, m_endLoc });
    ret->expr = std::move(expr);
    return ret;
}

//----------------------------------------
// If statement
//----------------------------------------

/**
 * IF
 *   = IfBlock
 *   { ELSE IF IfBlock }
 *   [ ELSE ThenBlock ]
 *   "END" "IF"
 *   .
 */
unique_ptr<AstIfStmt> Parser::kwIf() noexcept {
    auto start = m_token->range().Start;
    std::vector<AstIfStmt::Block> m_blocks;

    expect(TokenKind::If);
    m_blocks.emplace_back(ifBlock());

    if (match(TokenKind::EndOfStmt) && m_next->kind() == TokenKind::Else) {
        move();
    }

    while (accept(TokenKind::Else)) {
        if (accept(TokenKind::If)) {
            m_blocks.emplace_back(ifBlock());
        } else {
            m_blocks.emplace_back(thenBlock({}, nullptr));
        }

        if (match(TokenKind::EndOfStmt) && m_next->kind() == TokenKind::Else) {
            move();
        }
    }

    if (m_blocks.back().stmt->kind() == AstKind::StmtList) {
        expect(TokenKind::End);
        expect(TokenKind::If);
    }

    auto ast = AstIfStmt::create({ start, m_endLoc });
    ast->blocks = std::move(m_blocks);
    return ast;
}

/**
 * IfBlock
 *   = [ VAR { "," VAR } "," ] Expression "THEN" ThenBlock
 *   .
 */
AstIfStmt::Block Parser::ifBlock() noexcept {
    std::vector<unique_ptr<AstVarDecl>> decls;
    while (match(TokenKind::Var)) {
        decls.emplace_back(kwVar(nullptr));
        expect(TokenKind::Comma);
    }

    auto expr = expression(true);
    expect(TokenKind::Then);
    return thenBlock(std::move(decls), std::move(expr));
}

/**
 * ThenBlock
 *   =
 *   ( EoS StmtList
 *   | Statement
 *   )
 *   .
 */
[[nodiscard]] AstIfStmt::Block Parser::thenBlock(std::vector<unique_ptr<AstVarDecl>> decls, unique_ptr<AstExpr> expr) noexcept {
    unique_ptr<AstStmt> stmt;
    if (accept(TokenKind::EndOfStmt)) {
        stmt = stmtList();
    } else {
        stmt = statement();
    }
    return AstIfStmt::Block{ std::move(decls), nullptr, std::move(expr), std::move(stmt) };
}

//----------------------------------------
// FOR statement
//----------------------------------------

/**
 * FOR
 *   = "FOR" [ VAR { "," VAR } "," ]
 *     id [ "AS" TypeExpr ] "=" Expression "TO" Expression [ "STEP" expression ]
 *   ( "DO" Statement
 *   | <EoS> StatementList
 *     "NEXT" [ id ]
 *   )
 *   .
 */
[[nodiscard]] unique_ptr<AstForStmt> Parser::kwFor() noexcept {
    auto start = m_token->range().Start;
    expect(TokenKind::For);

    std::vector<unique_ptr<AstVarDecl>> decls;

    // [ VAR { "," VAR } "," ]
    while (match(TokenKind::Var)) {
        decls.emplace_back(kwVar(nullptr));
        expect(TokenKind::Comma);
    }

    // id [ "AS" TypeExpr ] "=" Expression
    auto idStart = m_token->range().Start;
    auto id = expect(TokenKind::Identifier);
    unique_ptr<AstTypeExpr> type;
    if (accept(TokenKind::As)) {
        type = typeExpr();
    }
    expect(TokenKind::Assign);
    auto expr = expression();
    auto iterator = AstVarDecl::create({ idStart, m_endLoc });
    iterator->expr = std::move(expr);
    iterator->typeExpr = std::move(type);
    iterator->id = std::get<StringRef>(id->getValue());

    // "TO" Expression [ "STEP" expression ]
    expect(TokenKind::To);
    auto limit = expression();
    unique_ptr<AstExpr> step;
    if (accept(TokenKind::Step)) {
        step = expression();
    }

    // "DO" statement ?
    unique_ptr<AstStmt> stmt;
    StringRef next;
    if (accept(TokenKind::Do)) {
        stmt = statement();
    } else {
        expect(TokenKind::EndOfStmt);
        stmt = stmtList();
        expect(TokenKind::Next);
        if (auto nextId = accept(TokenKind::Identifier)) {
            next = std::get<StringRef>(nextId->getValue());
        }
    }

    auto ast = AstForStmt::create({ start, m_endLoc });
    ast->decls = std::move(decls);
    ast->iterator = std::move(iterator);
    ast->limit = std::move(limit);
    ast->step = std::move(step);
    ast->stmt = std::move(stmt);
    ast->next = next;

    return ast;
}

//----------------------------------------
// Types
//----------------------------------------

unique_ptr<AstTypeExpr> Parser::typeExpr() noexcept {
    auto start = m_token->range().Start;
#define TYPE_KEYWORD(id, ...) case TokenKind::id:
    switch (m_token->kind()) {
        ALL_TYPES(TYPE_KEYWORD)
        {
            auto kind = move()->kind();
            auto type = AstTypeExpr::create({ start, m_endLoc });
            type->tokenKind = kind;
            return type;
        }
    default:
        error("Expected type got "_t + m_token->description());
    }
#undef TYPE_KEYWORD
}

//----------------------------------------
// Expressions
//----------------------------------------

/**
 * expression = factor { <Binary Op> expression } .
 */
unique_ptr<AstExpr> Parser::expression(bool commaAsAnd) noexcept {
    auto expr = factor();

    replace(TokenKind::Assign, TokenKind::Equal);
    if (commaAsAnd) {
        replace(TokenKind::Comma, TokenKind::CommaAnd);
    }
    if (m_token->isOperator()) {
        return expression(std::move(expr), 1, commaAsAnd);
    }

    return expr;
}

/**
 * factor = primary { <Right Unary Op> | "AS" TypeExpr } .
 */
unique_ptr<AstExpr> Parser::factor() noexcept {
    auto start = m_token->range().Start;
    auto expr = primary();

    while (true) {
        // <Right Unary Op>
        if (m_token->isUnary() && m_token->isRightToLeft()) {
            auto kind = move()->kind();
            auto unary = AstUnaryExpr::create({ start, m_endLoc });
            unary->expr = std::move(expr);
            unary->tokenKind = kind;
            expr = std::move(unary);
            continue;
        }

        // "AS" TypeExpr
        if (accept(TokenKind::As)) {
            auto type = typeExpr();
            auto cast = AstCastExpr::create({ start, m_endLoc });
            cast->expr = std::move(expr);
            cast->typeExpr = std::move(type);
            cast->implicit = false;
            expr = std::move(cast);
            continue;
        }
        break;
    }

    return expr;
}

/**
 * primary = literal
 *         | CallExpr
 *         | identifier
 *         | "(" expression ")"
 *         | <Left Unary Op> [ factor { <Binary Op> expression } ]
 *         | IfExpr
  *        .
 */
unique_ptr<AstExpr> Parser::primary() noexcept {
    if (m_token->isLiteral()) {
        return literal();
    }

    if (match(TokenKind::Identifier)) {
        if (m_next && *m_next == TokenKind::ParenOpen) {
            return callExpr();
        }
        return identifier();
    }

    if (accept(TokenKind::ParenOpen)) {
        auto expr = expression();
        expect(TokenKind::ParenClose);
        return expr;
    }

    if (match(TokenKind::If)) {
        return ifExpr();
    }

    replace(TokenKind::Minus, TokenKind::Negate);
    if (m_token->isUnary() && m_token->isLeftToRight()) {
        auto start = m_token->range().Start;
        auto prec = m_token->getPrecedence();
        auto kind = move()->kind();

        replace(TokenKind::Assign, TokenKind::Equal);
        auto expr = expression(factor(), prec);

        auto unary = AstUnaryExpr::create({ start, m_endLoc });
        unary->expr = std::move(expr);
        unary->tokenKind = kind;
        return unary;
    }

    error("expected primary");
}

/**
 * Recursievly climb operator precedence
 * https://en.wikipedia.org/wiki/Operator-precedence_parser#Precedence_climbing_method
 */
[[nodiscard]] unique_ptr<AstExpr> Parser::expression(unique_ptr<AstExpr> lhs, int precedence, bool commaAsAnd) noexcept {
    while (m_token->getPrecedence() >= precedence) {
        auto current = m_token->getPrecedence();
        auto kind = m_token->kind();
        move();

        auto rhs = factor();
        replace(TokenKind::Assign, TokenKind::Equal);
        if (commaAsAnd) {
            replace(TokenKind::Comma, TokenKind::CommaAnd);
        }

        while (m_token->getPrecedence() > current || (m_token->isRightToLeft() && m_token->getPrecedence() == current)) {
            rhs = expression(std::move(rhs), m_token->getPrecedence(), commaAsAnd);
        }

        auto left = AstBinaryExpr::create({ lhs->getRange().Start, m_endLoc });
        left->tokenKind = kind == TokenKind::CommaAnd ? TokenKind::LogicalAnd : kind;
        left->lhs = std::move(lhs);
        left->rhs = std::move(rhs);
        lhs = std::move(left);
    }
    return lhs;
}

/**
 * identExpr = identifier .
 */
unique_ptr<AstIdentExpr> Parser::identifier() noexcept {
    auto start = m_token->range().Start;
    auto id = expect(TokenKind::Identifier);

    auto ident = AstIdentExpr::create({ start, m_endLoc });
    ident->id = std::get<StringRef>(id->getValue());
    return ident;
}

/**
 * callExpr = identifier "(" argList ")" .
 */
unique_ptr<AstCallExpr> Parser::callExpr() noexcept {
    auto start = m_token->range().Start;

    auto id = identifier();
    expect(TokenKind::ParenOpen);
    auto args = expressionList();
    expect(TokenKind::ParenClose);

    auto call = AstCallExpr::create({ start, m_endLoc });
    call->identExpr = std::move(id);
    call->argExprs = std::move(args);
    return call;
}

/**
 * IfExpr = "IF" expr "THEN" expr "ELSE" expr .
 */
unique_ptr<AstIfExpr> Parser::ifExpr() noexcept {
    auto start = m_token->range().Start;

    expect(TokenKind::If);
    auto expr = expression(true);
    expect(TokenKind::Then);
    auto trueExpr = expression();
    expect(TokenKind::Else);
    auto falseExpr = expression();

    auto ast = AstIfExpr::create({ start, m_endLoc });
    ast->expr = std::move(expr);
    ast->trueExpr = std::move(trueExpr);
    ast->falseExpr = std::move(falseExpr);
    return ast;
}

/**
 * literal = stringLiteral
 *         | IntegerLiteral
 *         | FloatingPointLiteral
 *         | "TRUE"
 *         | "FALSE"
 *         | "NULL"
 *         .
 */
unique_ptr<AstLiteralExpr> Parser::literal() noexcept {
    auto lit = AstLiteralExpr::create(m_token->range());
    lit->value = move()->getValue();
    return lit;
}

/**
 * Parse comma separated list of expressionds
 */
std::vector<unique_ptr<AstExpr>> Parser::expressionList() noexcept {
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

bool Parser::isValid() const noexcept {
    return *m_token != TokenKind::EndOfFile;
}

void Parser::replace(TokenKind what, TokenKind with) noexcept {
    if (match(what)) {
        m_token = m_token->convert(with);
    }
}

bool Parser::match(TokenKind kind) const noexcept {
    return *m_token == kind;
}

unique_ptr<Token> Parser::accept(TokenKind kind) noexcept {
    if (match(kind)) {
        return move();
    }
    return nullptr;
}

unique_ptr<Token> Parser::expect(TokenKind kind) noexcept {
    if (!match(kind)) {
        error("Expected '"_t
            + Token::description(kind)
            + "' got '"
            + m_token->description()
            + "'");
    }
    return move();
}

unique_ptr<Token> Parser::move() noexcept {
    auto current = std::move(m_token);
    m_endLoc = current->range().End;
    m_token = std::move(m_next);
    m_next = m_lexer->next();
    return current;
}

[[noreturn]] void Parser::error(const Twine& message) noexcept {
    string output;
    llvm::raw_string_ostream stream{ output };

    m_context.getSourceMrg().PrintMessage(
        stream,
        m_token->range().Start,
        llvm::SourceMgr::DK_Error,
        message,
        m_token->range());

    fatalError(output, false);
}
