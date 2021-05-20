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

    return AstModule::create(
        m_fileId,
        stmts->range,
        m_isMain,
        std::move(stmts));
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

    return AstStmtList::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(stms));
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

    return AstAttributeList::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(attribs));
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

    return AstAttribute::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(id),
        std::move(args));
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
    auto start = attribs == nullptr ? m_token->range().Start : attribs->range.Start;

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

    return AstVarDecl::create(
        llvm::SMRange{ start, m_endLoc },
        std::get<StringRef>(id->getValue()),
        std::move(attribs),
        std::move(type),
        std::move(expr));
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
    auto start = attribs == nullptr ? m_token->range().Start : attribs->range.Start;
    expect(TokenKind::Declare);
    return funcSignature(start, std::move(attribs), false);
}

/**
 * FuncSignature
 *     = "FUNCTION" id [ "(" [ FuncParamList ] ")" ] "AS" TypeExpr
 *     | "SUB" id [ "(" FuncParamList ")" ]
 *     .
 */
unique_ptr<AstFuncDecl> Parser::funcSignature(llvm::SMLoc start, unique_ptr<AstAttributeList> attribs, bool hasImpl) noexcept {
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

    return AstFuncDecl::create(
        llvm::SMRange{ start, m_endLoc },
        std::get<StringRef>(id->getValue()),
        std::move(attribs),
        std::move(params),
        isVariadic,
        std::move(ret),
        hasImpl);
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

    return AstFuncParamDecl::create(
        llvm::SMRange{ start, m_endLoc },
        std::get<StringRef>(id->getValue()),
        nullptr,
        std::move(type));
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
    auto lhs = expression({});
    expect(TokenKind::Assign);
    auto rhs = expression();

    return AstAssignStmt::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(lhs),
        std::move(rhs));
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

    auto call = AstCallExpr::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(id),
        std::move(args));

    return AstExprStmt::create(call->range, std::move(call));
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

    auto start = attribs == nullptr ? m_token->range().Start : attribs->range.Start;
    auto decl = funcSignature(start, std::move(attribs), true);
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

    return AstFuncStmt::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(decl),
        std::move(stmts));
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

    return AstReturnStmt::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(expr));
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

    if (m_blocks.back().stmt->kind == AstKind::StmtList) {
        expect(TokenKind::End);
        expect(TokenKind::If);
    }

    return AstIfStmt::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(m_blocks));
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

    auto expr = expression(ExprFlags::Default | ExprFlags::CommaAsAnd);
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
    auto iterator = AstVarDecl::create(
        llvm::SMRange{ idStart, m_endLoc },
        std::get<StringRef>(id->getValue()),
        nullptr,
        std::move(type),
        std::move(expr));

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

    return AstForStmt::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(decls),
        std::move(iterator),
        std::move(limit),
        std::move(step),
        std::move(stmt),
        next);
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
            return AstTypeExpr::create(
                llvm::SMRange{ start, m_endLoc },
                kind);
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
unique_ptr<AstExpr> Parser::expression(ExprFlags flags) noexcept {
    RESTORE_ON_EXIT(m_exprFlags);
    m_exprFlags = flags;

    auto expr = factor();

    if ((flags & ExprFlags::AssignAsEqual) != 0) {
        replace(TokenKind::Assign, TokenKind::Equal);
    }
    if ((flags & ExprFlags::CommaAsAnd) != 0) {
        replace(TokenKind::Comma, TokenKind::CommaAnd);
    }
    if (m_token->isOperator()) {
        return expression(std::move(expr), 1);
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
            auto unary = AstUnaryExpr::create(
                llvm::SMRange{ start, m_endLoc },
                kind,
                std::move(expr));
            expr = std::move(unary);
            continue;
        }

        // "AS" TypeExpr
        if (accept(TokenKind::As)) {
            auto type = typeExpr();
            auto cast = AstCastExpr::create(
                llvm::SMRange{ start, m_endLoc },
                std::move(expr),
                std::move(type),
                false);
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

        if ((m_exprFlags & ExprFlags::AssignAsEqual) != 0) {
            replace(TokenKind::Assign, TokenKind::Equal);
        }
        auto expr = expression(factor(), prec);

        return AstUnaryExpr::create(
            llvm::SMRange{ start, m_endLoc },
            kind,
            std::move(expr));
    }

    error("expected primary");
}

/**
 * Recursievly climb operator precedence
 * https://en.wikipedia.org/wiki/Operator-precedence_parser#Precedence_climbing_method
 */
[[nodiscard]] unique_ptr<AstExpr> Parser::expression(unique_ptr<AstExpr> lhs, int precedence) noexcept {
    while (m_token->getPrecedence() >= precedence) {
        auto current = m_token->getPrecedence();
        auto kind = m_token->kind();
        move();

        auto rhs = factor();
        if ((m_exprFlags & ExprFlags::AssignAsEqual) != 0) {
            replace(TokenKind::Assign, TokenKind::Equal);
        }
        if ((m_exprFlags & ExprFlags::CommaAsAnd) != 0) {
            replace(TokenKind::Comma, TokenKind::CommaAnd);
        }

        while (m_token->getPrecedence() > current || (m_token->isRightToLeft() && m_token->getPrecedence() == current)) {
            rhs = expression(std::move(rhs), m_token->getPrecedence());
        }

        auto left = AstBinaryExpr::create(
            llvm::SMRange{ lhs->range.Start, m_endLoc },
            kind == TokenKind::CommaAnd ? TokenKind::LogicalAnd : kind,
            std::move(lhs),
            std::move(rhs));
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

    return AstIdentExpr::create(
        llvm::SMRange{ start, m_endLoc },
        std::get<StringRef>(id->getValue()));
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

    return AstCallExpr::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(id),
        std::move(args));
}

/**
 * IfExpr = "IF" expr "THEN" expr "ELSE" expr .
 */
unique_ptr<AstIfExpr> Parser::ifExpr() noexcept {
    auto start = m_token->range().Start;

    expect(TokenKind::If);
    auto expr = expression(ExprFlags::Default | ExprFlags::CommaAsAnd);
    expect(TokenKind::Then);
    auto trueExpr = expression();
    expect(TokenKind::Else);
    auto falseExpr = expression();

    return AstIfExpr::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(expr),
        std::move(trueExpr),
        std::move(falseExpr));
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
    return AstLiteralExpr::create(
        m_token->range(),
        move()->getValue());
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
