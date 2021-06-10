//
// Created by Albert Varaksin on 03/07/2020.
//
#include "Parser.hpp"
#include "Ast/Ast.hpp"
#include "Driver/Context.hpp"
#include "Lexer/Lexer.hpp"
#include "Lexer/Token.hpp"
#include "Type/Type.hpp"
#include <limits>
using namespace lbc;

Parser::Parser(Context& context, unsigned int fileId, bool isMain)
: m_context{ context },
  m_fileId{ fileId },
  m_isMain{ isMain },
  m_scope{ Scope::Root } {
    setupLexer();
}

Parser::~Parser() noexcept = default;

/**
 * Module
 *   = StmtList
 *   .
 */
unique_ptr<AstModule> Parser::parse() {
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
unique_ptr<AstStmtList> Parser::stmtList() {
    constexpr auto isTerminator = [](const Token& token) {
        switch (token.kind()) {
        case TokenKind::End:
        case TokenKind::Else:
        case TokenKind::Next:
        case TokenKind::Loop:
            return true;
        default:
            return false;
        }
    };

    auto start = m_token.range().Start;
    std::vector<unique_ptr<AstStmt>> stms;
    while (isValid() && !isTerminator(m_token)) {
        stms.emplace_back(statement());
        expect(TokenKind::EndOfStmt);
        advance();
    }

    return AstStmtList::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(stms));
}

/**
 * Statement
 *   = Declaration
 *   | IMPORT
 *   | IfStmt
 *   | ForStmt
 *   | DoLoopStmt
 *   | RETURN
 *   | EXIT
 *   | CONTINUE
 *   | Expression
 *   .
 */
unique_ptr<AstStmt> Parser::statement() {
    if (auto decl = declaration()) {
        return decl;
    }

    if (m_scope == Scope::Root) {
        if (match(TokenKind::Import)) {
            return kwImport();
        }
        if (!m_isMain) {
            error("Only declarations are allowed at root level. Got "_t + m_token.description());
        }
    }

    switch (m_token.kind()) {
    case TokenKind::Return:
        return kwReturn();
    case TokenKind::If:
        return kwIf();
    case TokenKind::For:
        return kwFor();
    case TokenKind::Do:
        return kwDo();
    case TokenKind::Continue:
        return kwContinue();
    case TokenKind::Exit:
        return kwExit();
    default:
        break;
    }

    auto expr = expression(ExprFlags::UseAssign | ExprFlags::CallWithoutParens);
    return AstExprStmt::create(expr->range, std::move(expr));
}

/**
 * IMPORT
 *   = "IMPORT" id
 *   .
 */
unique_ptr<AstStmtList> Parser::kwImport() {
    // assume m_token == Import
    advance();

    expect(TokenKind::Identifier);
    auto id = m_token.lexeme();
    auto start = m_token.range().Start;
    advance();

    // Imported file
    auto source = m_context.getCompilerDir() / "lib" / (id + ".bas").str();
    if (!m_imports.insert(source.string()).second) {
        return AstStmtList::create(
            llvm::SMRange{ start, m_endLoc },
            std::vector<unique_ptr<AstStmt>>{});
    }
    if (!fs::exists(source)) {
        error("Import '"_t + id + "' not found");
    }

    // Load import into Source Mgr
    string included;
    auto ID = m_context.getSourceMrg().AddIncludeFile(
        source.string(),
        start,
        included);
    if (ID == ~0U) {
        fatalError("Failed to load '"_t + source.string() + "'");
    }

    // push state, parse the import
    pushState();
    m_fileId = ID;
    m_isMain = false;
    setupLexer();
    auto stmts = stmtList();
    popState();

    // done
    return stmts;
}

/**
 * Declaration
 *   = [
 *     [ AttributeList ]
 *     ( VAR
 *     | DECLARE
 *     | FUNCTION
 *     | SUB
 *     )
 *   ]
 *   .
 */
unique_ptr<AstStmt> Parser::declaration() {
    auto attribs = attributeList();

    switch (m_token.kind()) {
    case TokenKind::Var:
        return kwVar(std::move(attribs));
    case TokenKind::Declare:
        return kwDeclare(std::move(attribs));
    case TokenKind::Function:
    case TokenKind::Sub:
        return kwFunction(std::move(attribs));
    case TokenKind::Type:
        return kwType(std::move(attribs));
    default:
        break;
    }

    if (attribs) {
        error("Expected SUB, FUNCTION, DECLARE or VAR got '"_t
            + m_token.description()
            + "'");
    }
    return nullptr;
}

//----------------------------------------
// Attributes
//----------------------------------------

/**
 *  AttributeList = [ '[' Attribute { ','  Attribute } ']' ].
 */
unique_ptr<AstAttributeList> Parser::attributeList() {
    if (!match(TokenKind::BracketOpen)) {
        return nullptr;
    }

    auto start = m_token.range().Start;
    advance();

    std::vector<unique_ptr<AstAttribute>> attribs;

    do {
        attribs.emplace_back(attribute());
    } while (accept(TokenKind::Comma));

    expect(TokenKind::BracketClose);
    advance();

    return AstAttributeList::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(attribs));
}

/**
 * Attribute
 *   = IdentExpr [ AttributeArgList ]
 *   .
 */
unique_ptr<AstAttribute> Parser::attribute() {
    auto start = m_token.range().Start;

    auto id = identifier();
    std::vector<unique_ptr<AstLiteralExpr>> args;
    if (m_token.isOneOf(TokenKind::Assign, TokenKind::ParenOpen)) {
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
std::vector<unique_ptr<AstLiteralExpr>> Parser::attributeArgList() {
    std::vector<unique_ptr<AstLiteralExpr>> args;
    if (accept(TokenKind::Assign)) {
        args.emplace_back(literal());
    } else if (accept(TokenKind::ParenOpen)) {
        while (isValid() && m_token != TokenKind::ParenClose) {
            args.emplace_back(literal());
            if (!accept(TokenKind::Comma)) {
                break;
            }
        }
        expect(TokenKind::ParenClose);
        advance();
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
unique_ptr<AstVarDecl> Parser::kwVar(unique_ptr<AstAttributeList> attribs) {
    // assume m_token == VAR
    auto start = attribs == nullptr ? m_token.range().Start : attribs->range.Start;
    advance();

    expect(TokenKind::Identifier);
    auto id = m_token.getStringValue();
    advance();

    unique_ptr<AstTypeExpr> type;
    unique_ptr<AstExpr> expr;

    if (accept(TokenKind::As)) {
        type = typeExpr();
        if (accept(TokenKind::Assign)) {
            expr = expression();
        }
    } else {
        expect(TokenKind::Assign);
        advance();

        expr = expression();
    }

    return AstVarDecl::create(
        llvm::SMRange{ start, m_endLoc },
        id,
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
unique_ptr<AstFuncDecl> Parser::kwDeclare(unique_ptr<AstAttributeList> attribs) {
    // assume m_token == DECLARE
    if (m_scope != Scope::Root) {
        error("Nested declarations not allowed");
    }
    auto start = attribs == nullptr ? m_token.range().Start : attribs->range.Start;
    advance();

    return funcSignature(start, std::move(attribs), false);
}

/**
 * FuncSignature
 *     = "FUNCTION" id [ "(" [ FuncParamList ] ")" ] "AS" TypeExpr
 *     | "SUB" id [ "(" FuncParamList ")" ]
 *     .
 */
unique_ptr<AstFuncDecl> Parser::funcSignature(llvm::SMLoc start, unique_ptr<AstAttributeList> attribs, bool hasImpl) {
    bool isFunc = accept(TokenKind::Function);
    if (!isFunc) {
        expect(TokenKind::Sub);
        advance();
    }

    expect(TokenKind::Identifier);
    auto id = m_token.getStringValue();
    advance();

    bool isVariadic = false;
    std::vector<unique_ptr<AstFuncParamDecl>> params;
    if (accept(TokenKind::ParenOpen)) {
        params = funcParamList(isVariadic);
        expect(TokenKind::ParenClose);
        advance();
    }

    unique_ptr<AstTypeExpr> ret;
    if (isFunc) {
        expect(TokenKind::As);
        advance();
        ret = typeExpr();
    }

    return AstFuncDecl::create(
        llvm::SMRange{ start, m_endLoc },
        id,
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
std::vector<unique_ptr<AstFuncParamDecl>> Parser::funcParamList(bool& isVariadic) {
    std::vector<unique_ptr<AstFuncParamDecl>> params;
    while (isValid() && m_token != TokenKind::ParenClose) {
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
unique_ptr<AstFuncParamDecl> Parser::funcParam() {
    auto start = m_token.range().Start;

    expect(TokenKind::Identifier);
    auto id = m_token.getStringValue();
    advance();

    expect(TokenKind::As);
    advance();
    auto type = typeExpr();

    return AstFuncParamDecl::create(
        llvm::SMRange{ start, m_endLoc },
        id,
        nullptr,
        std::move(type));
}

//----------------------------------------
// TYPE
//----------------------------------------

/**
 * TYPE
 *   = "TYPE" id EoS
 *     typeDeclList
 *     "END" "TYPE"
 *   .
 */
unique_ptr<AstTypeDecl> Parser::kwType(unique_ptr<AstAttributeList> attribs) {
    // assume m_token == TYPE
    auto start = m_token.range().Start;
    advance();

    expect(TokenKind::Identifier);
    auto id = m_token.getStringValue();
    advance();

    expect(TokenKind::EndOfStmt);
    advance();

    auto decls = typeDeclList();

    expect(TokenKind::End);
    advance();

    expect(TokenKind::Type);
    advance();

    return AstTypeDecl::create(
        llvm::SMRange{ start, m_endLoc },
        id,
        std::move(attribs),
        std::move(decls));
}

/**
 * typeDeclList
 *   = { [ AttributeList ] typeMember EoS }
 *   .
 */
std::vector<unique_ptr<AstDecl>> Parser::typeDeclList() {
    std::vector<unique_ptr<AstDecl>> decls;

    while (true) {
        auto attribs = attributeList();
        if (attribs && expect(TokenKind::Identifier) || match(TokenKind::Identifier)) {
            decls.emplace_back(typeMember(std::move(attribs)));
            expect(TokenKind::EndOfStmt);
            advance();
            continue;
        }
        break;
    }

    return decls;
}

/**
 * typeMember
 *   = id "AS" TypeExpr
 *   .
 */
unique_ptr<AstDecl> Parser::typeMember(unique_ptr<AstAttributeList> attribs) {
    // assume m_token == Identifier
    auto start = m_token.range().Start;
    auto id = m_token.getStringValue();
    advance();

    expect(TokenKind::As);
    advance();

    auto type = typeExpr();

    return AstVarDecl::create(
        llvm::SMRange{ start, m_endLoc },
        id,
        std::move(attribs),
        std::move(type),
        nullptr);
}

//----------------------------------------
// Call
//----------------------------------------

/**
 *  FUNCTION = funcSignature <EoS>
 *             stmtList
 *             "END" ("FUNCTION" | "SUB")
 */
unique_ptr<AstFuncStmt> Parser::kwFunction(unique_ptr<AstAttributeList> attribs) {
    if (m_scope != Scope::Root) {
        error("Nested SUBs/FUNCTIONs not allowed");
    }

    auto start = attribs == nullptr ? m_token.range().Start : attribs->range.Start;
    auto decl = funcSignature(start, std::move(attribs), true);
    expect(TokenKind::EndOfStmt);
    advance();

    RESTORE_ON_EXIT(m_scope);
    m_scope = Scope::Function;

    auto stmts = stmtList();

    expect(TokenKind::End);
    advance();

    if (decl->retTypeExpr) {
        expect(TokenKind::Function);
        advance();
    } else {
        expect(TokenKind::Sub);
        advance();
    }

    return AstFuncStmt::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(decl),
        std::move(stmts));
}

/**
 * RETURN = "RETURN" [ expression ] .
 */
unique_ptr<AstStmt> Parser::kwReturn() {
    // assume m_token == RETURN
    if (m_scope == Scope::Root && !m_isMain) {
        error("Unexpected RETURN outside SUB / FUNCTION body");
    }
    auto start = m_token.range().Start;
    advance();

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
unique_ptr<AstIfStmt> Parser::kwIf() {
    // assume m_token == IF
    auto start = m_token.range().Start;
    advance();

    std::vector<AstIfStmtBlock> m_blocks;
    m_blocks.emplace_back(ifBlock());

    if (match(TokenKind::EndOfStmt)) {
        Token next;
        m_lexer->peek(next);
        if (next.kind() == TokenKind::Else) {
            advance();
        }
    }

    while (accept(TokenKind::Else)) {
        if (accept(TokenKind::If)) {
            m_blocks.emplace_back(ifBlock());
        } else {
            m_blocks.emplace_back(thenBlock({}, nullptr));
        }

        if (match(TokenKind::EndOfStmt)) {
            Token next;
            m_lexer->peek(next);
            if (next.kind() == TokenKind::Else) {
                advance();
            }
        }
    }

    if (m_blocks.back().stmt->kind == AstKind::StmtList) {
        expect(TokenKind::End);
        advance();

        expect(TokenKind::If);
        advance();
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
AstIfStmtBlock Parser::ifBlock() {
    std::vector<unique_ptr<AstVarDecl>> decls;
    while (match(TokenKind::Var)) {
        decls.emplace_back(kwVar(nullptr));
        expect(TokenKind::Comma);
        advance();
    }

    auto expr = expression(ExprFlags::CommaAsAnd);
    expect(TokenKind::Then);
    advance();

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
[[nodiscard]] AstIfStmtBlock Parser::thenBlock(std::vector<unique_ptr<AstVarDecl>> decls, unique_ptr<AstExpr> expr) {
    unique_ptr<AstStmt> stmt;
    if (accept(TokenKind::EndOfStmt)) {
        stmt = stmtList();
    } else {
        stmt = statement();
    }
    return AstIfStmtBlock{ std::move(decls), nullptr, std::move(expr), std::move(stmt) };
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
[[nodiscard]] unique_ptr<AstForStmt> Parser::kwFor() {
    // assume m_token == FOR
    auto start = m_token.range().Start;
    advance();

    std::vector<unique_ptr<AstVarDecl>> decls;

    // [ VAR { "," VAR } "," ]
    while (match(TokenKind::Var)) {
        decls.emplace_back(kwVar(nullptr));
        expect(TokenKind::Comma);
        advance();
    }

    // id [ "AS" TypeExpr ] "=" Expression
    auto idStart = m_token.range().Start;
    expect(TokenKind::Identifier);
    auto id = m_token.getStringValue();
    advance();

    unique_ptr<AstTypeExpr> type;
    if (accept(TokenKind::As)) {
        type = typeExpr();
    }

    expect(TokenKind::Assign);
    advance();

    auto expr = expression();
    auto iterator = AstVarDecl::create(
        llvm::SMRange{ idStart, m_endLoc },
        id,
        nullptr,
        std::move(type),
        std::move(expr));

    // "TO" Expression [ "STEP" expression ]
    expect(TokenKind::To);
    advance();

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
        advance();

        stmt = stmtList();

        expect(TokenKind::Next);
        advance();

        if (match(TokenKind::Identifier)) {
            next = m_token.getStringValue();
            advance();
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
// DO ... LOOP statement
//----------------------------------------

/**
 * DO = "DO" [ VAR { "," VAR } ]
 *    ( EndOfStmt StmtList "LOOP" [ LoopCondition ]
 *    | [ LoopCondition ] ( EoS StmtList "LOOP" | "DO" Statement )
 *    )
 *    .
 * LoopCondition
 *   = ("UNTIL" | "WHILE") expression
 *   .
 */
[[nodiscard]] unique_ptr<AstDoLoopStmt> Parser::kwDo() {
    // assume m_token == DO
    auto start = m_token.range().Start;
    advance();

    auto condition = AstDoLoopStmt::Condition::None;
    unique_ptr<AstStmt> stmt;
    unique_ptr<AstExpr> expr;
    std::vector<unique_ptr<AstVarDecl>> decls;

    // [ VAR { "," VAR } ]
    auto acceptComma = false;
    while (match(TokenKind::Var) || (acceptComma && accept(TokenKind::Comma))) {
        acceptComma = true;
        decls.emplace_back(kwVar(nullptr));

    }

    // ( EoS StmtList "LOOP" [ Condition ]
    if (accept(TokenKind::EndOfStmt)) {
        stmt = stmtList();

        expect(TokenKind::Loop);
        advance();

        // [ Condition ]
        if (accept(TokenKind::Until)) {
            condition = AstDoLoopStmt::Condition::PostUntil;
            expr = expression(ExprFlags::CommaAsAnd);
        } else if (accept(TokenKind::While)) {
            condition = AstDoLoopStmt::Condition::PostWhile;
            expr = expression(ExprFlags::CommaAsAnd);
        }
    } else {
        // [ Condition ]
        if (accept(TokenKind::Until)) {
            condition = AstDoLoopStmt::Condition::PreUntil;
            expr = expression(ExprFlags::CommaAsAnd);
        } else if (accept(TokenKind::While)) {
            condition = AstDoLoopStmt::Condition::PreWhile;
            expr = expression(ExprFlags::CommaAsAnd);
        }

        // EoS StmtList "LOOP"
        if (accept(TokenKind::EndOfStmt)) {
            stmt = stmtList();

            expect(TokenKind::Loop);
            advance();
        }
        // "DO" Statement
        else {
            expect(TokenKind::Do);
            advance();

            stmt = statement();
        }
    }

    return AstDoLoopStmt::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(decls),
        condition,
        std::move(expr),
        std::move(stmt));
}

//----------------------------------------
// Branching
//----------------------------------------

/**
 * CONTINUE
 *   = "CONTINUE" { "FOR" }
 *   .
 */
unique_ptr<AstControlFlowBranch> Parser::kwContinue() {
    // assume m_token == CONTINUE
    auto start = m_token.range().Start;
    advance();

    std::vector<ControlFlowStatement> returnControl;

    while (true) {
        switch (m_token.kind()) {
        case TokenKind::For:
            advance();
            returnControl.emplace_back(ControlFlowStatement::For);
            continue;
        case TokenKind::Do:
            advance();
            returnControl.emplace_back(ControlFlowStatement::Do);
            continue;
        default:
            break;
        }
        break;
    }

    return AstControlFlowBranch::create(
        llvm::SMRange{ start, m_endLoc },
        AstControlFlowBranch::Action::Continue,
        std::move(returnControl));
}

/**
 * EXIT
 *   = "EXIT" { "FOR" }
 *   .
 */
unique_ptr<AstControlFlowBranch> Parser::kwExit() {
    // assume m_token == EXIT
    auto start = m_token.range().Start;
    advance();

    std::vector<ControlFlowStatement> returnControl;

    while (true) {
        switch (m_token.kind()) {
        case TokenKind::For:
            advance();
            returnControl.emplace_back(ControlFlowStatement::For);
            continue;
        case TokenKind::Do:
            advance();
            returnControl.emplace_back(ControlFlowStatement::Do);
            continue;
        default:
            break;
        }
        break;
    }

    return AstControlFlowBranch::create(
        llvm::SMRange{ start, m_endLoc },
        AstControlFlowBranch::Action::Exit,
        std::move(returnControl));
}

//----------------------------------------
// Types
//----------------------------------------

/**
 * TypeExpr = ( identExpr | Any ) { "PTR" } .
 */
unique_ptr<AstTypeExpr> Parser::typeExpr() {
    auto start = m_token.range().Start;
    auto kind = m_token.kind();

    unique_ptr<AstIdentExpr> ident;
    if (match(TokenKind::Any) || m_token.isTypeKeyword()) {
        advance();
    } else {
        ident = identifier();
    }

    auto deref = 0;
    while (accept(TokenKind::Ptr)) {
        deref++;
    }

    return AstTypeExpr::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(ident),
        kind,
        deref);
}

//----------------------------------------
// Expressions
//----------------------------------------

/**
 * expression = factor { <Binary Op> expression }
 *            . [ ArgumentList ]
 */
unique_ptr<AstExpr> Parser::expression(ExprFlags flags) {
    RESTORE_ON_EXIT(m_exprFlags);
    m_exprFlags = flags;

    auto expr = factor();

    if ((flags & ExprFlags::UseAssign) == 0) {
        replace(TokenKind::Assign, TokenKind::Equal);
    }
    if ((flags & ExprFlags::CommaAsAnd) != 0) {
        replace(TokenKind::Comma, TokenKind::CommaAnd);
    }
    if (m_token.isOperator()) {
        expr = expression(std::move(expr), 1);
    }

    if ((m_exprFlags & ExprFlags::CallWithoutParens) != 0 && !match(TokenKind::EndOfStmt)) {
        if (m_token.is(TokenKind::Identifier) || m_token.isLiteral() || m_token.isUnary()) {
            auto start = expr->range.Start;
            auto args = expressionList();

            return AstCallExpr::create(
                llvm::SMRange{ start, m_endLoc },
                std::move(expr),
                std::move(args));
        }
    }

    return expr;
}

/**
 * factor = primary { <Right Unary Op> | "AS" TypeExpr } .
 */
unique_ptr<AstExpr> Parser::factor() {
    auto start = m_token.range().Start;
    auto expr = primary();

    while (true) {
        // <Right Unary Op>
        if (m_token.isUnary() && m_token.isRightToLeft()) {
            auto kind = m_token.kind();
            advance();

            expr = unary({ start, m_endLoc }, kind, std::move(expr));
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
unique_ptr<AstExpr> Parser::primary() {
    if (m_token.isLiteral()) {
        return literal();
    }

    // TODO callExpr should be resolved in the expression
    if (match(TokenKind::Identifier)) {
        Token next;
        m_lexer->peek(next);
        if (next == TokenKind::ParenOpen) {
            return callExpr();
        }
        return identifier();
    }

    if (accept(TokenKind::ParenOpen)) {
        auto expr = expression();
        expect(TokenKind::ParenClose);
        advance();
        return expr;
    }

    if (match(TokenKind::If)) {
        return ifExpr();
    }

    replace(TokenKind::Minus, TokenKind::Negate);
    replace(TokenKind::Multiply, TokenKind::Dereference);
    if (m_token.isUnary() && m_token.isLeftToRight()) {
        auto start = m_token.range().Start;
        auto prec = m_token.getPrecedence();
        auto kind = m_token.kind();
        advance();

        if ((m_exprFlags & ExprFlags::UseAssign) == 0) {
            replace(TokenKind::Assign, TokenKind::Equal);
        }
        auto expr = expression(factor(), prec);

        return unary({ start, m_endLoc }, kind, std::move(expr));
    }

    error("expected primary");
}

unique_ptr<AstExpr> Parser::unary(llvm::SMRange range, TokenKind op, unique_ptr<AstExpr> expr) {
    switch (op) {
    case TokenKind::Dereference:
        return AstDereference::create(range, std::move(expr));
    case TokenKind::AddressOf:
        return AstAddressOf::create(range, std::move(expr));
    default:
        return AstUnaryExpr::create(range, op, std::move(expr));
    }
}

unique_ptr<AstExpr> Parser::binary(llvm::SMRange range, TokenKind op, unique_ptr<AstExpr> lhs, unique_ptr<AstExpr> rhs) {
    switch (op) {
    case TokenKind::CommaAnd:
        return AstBinaryExpr::create(range, TokenKind::LogicalAnd, std::move(lhs), std::move(rhs));
    case TokenKind::Assign:
        return AstAssignExpr::create(range, std::move(lhs), std::move(rhs));
    case TokenKind::MemberAccess:
        return AstMemberAccess::create(range, std::move(lhs), std::move(rhs));
    default:
        return AstBinaryExpr::create(range, op, std::move(lhs), std::move(rhs));
    }
}

/**
 * Recursievly climb operator precedence
 * https://en.wikipedia.org/wiki/Operator-precedence_parser#Precedence_climbing_method
 */
unique_ptr<AstExpr> Parser::expression(unique_ptr<AstExpr> lhs, int precedence) {
    while (m_token.getPrecedence() >= precedence) {
        auto current = m_token.getPrecedence();
        auto kind = m_token.kind();
        advance();

        auto rhs = factor();
        if ((m_exprFlags & ExprFlags::UseAssign) == 0) {
            replace(TokenKind::Assign, TokenKind::Equal);
        }
        if ((m_exprFlags & ExprFlags::CommaAsAnd) != 0) {
            replace(TokenKind::Comma, TokenKind::CommaAnd);
        }

        while (m_token.getPrecedence() > current || (m_token.isRightToLeft() && m_token.getPrecedence() == current)) {
            rhs = expression(std::move(rhs), m_token.getPrecedence());
        }

        auto start = lhs->range.Start;
        lhs = binary({ start, m_endLoc }, kind, std::move(lhs), std::move(rhs));
    }
    return lhs;
}

/**
 * IdentExpr
 *   = id
 *   .
 */
unique_ptr<AstIdentExpr> Parser::identifier() {
    auto start = m_token.range().Start;
    expect(TokenKind::Identifier);
    auto name = m_token.getStringValue();
    advance();

    return AstIdentExpr::create(
        llvm::SMRange{ start, m_endLoc },
        name);
}

/**
 * callExpr = identifier "(" argList ")" .
 */
unique_ptr<AstCallExpr> Parser::callExpr() {
    auto start = m_token.range().Start;

    auto id = identifier();

    expect(TokenKind::ParenOpen);
    advance();

    auto args = expressionList();

    expect(TokenKind::ParenClose);
    advance();

    return AstCallExpr::create(
        llvm::SMRange{ start, m_endLoc },
        std::move(id),
        std::move(args));
}

/**
 * IfExpr = "IF" expr "THEN" expr "ELSE" expr .
 */
unique_ptr<AstIfExpr> Parser::ifExpr() {
    // assume m_token == IF
    auto start = m_token.range().Start;
    advance();

    auto expr = expression(ExprFlags::CommaAsAnd);

    expect(TokenKind::Then);
    advance();

    auto trueExpr = expression();

    expect(TokenKind::Else);
    advance();

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
unique_ptr<AstLiteralExpr> Parser::literal() {
    auto value = m_token.getValue();
    advance();

    return AstLiteralExpr::create(
        m_token.range(),
        value);
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

void Parser::setupLexer() {
    m_lexer = make_unique<Lexer>(m_context, m_fileId);
    m_lexer->next(m_token);
    m_endLoc = m_token.range().End;
}

void Parser::pushState() {
    m_stateStack.emplace_back(State{
        m_fileId,
        m_isMain,
        m_scope,
        std::move(m_lexer),
        m_token,
        m_endLoc,
        m_exprFlags });
}

void Parser::popState() {
    auto& state = m_stateStack.back();
    m_fileId = state.fileId;
    m_isMain = state.isMain;
    m_scope = state.scope;
    m_lexer = std::move(state.lexer);
    m_token = state.token;
    m_endLoc = state.endLoc;
    m_exprFlags = state.exprFlags;
    m_stateStack.pop_back();
}

void Parser::replace(TokenKind what, TokenKind with) noexcept {
    if (match(what)) {
        m_token.setKind(with);
    }
}

bool Parser::accept(TokenKind kind) {
    if (!match(kind)) {
        return false;
    }
    advance();
    return true;
}

bool Parser::expect(TokenKind kind) noexcept {
    if (match(kind)) {
        return true;
    }

    error("Expected '"_t
        + Token::description(kind)
        + "' got '"
        + m_token.description()
        + "'");
}

void Parser::advance() {
    m_endLoc = m_token.range().End;
    m_lexer->next(m_token);
}

void Parser::error(const Twine& message) {
    string output;
    llvm::raw_string_ostream stream{ output };

    m_context.getSourceMrg().PrintMessage(
        stream,
        m_token.range().Start,
        llvm::SourceMgr::DK_Error,
        message,
        m_token.range());

    fatalError(output, false);
}
