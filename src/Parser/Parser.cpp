//
// Created by Albert Varaksin on 03/07/2020.
//
#include "Parser.hpp"
#include "Ast/Ast.hpp"
#include "Diag/DiagnosticEngine.hpp"
#include "Driver/CompileOptions.hpp"
#include "Driver/Context.hpp"
#include "Lexer/Lexer.hpp"
#include "Lexer/Token.hpp"
#include "Type/Type.hpp"
using namespace lbc;

Parser::Parser(Context& context, unsigned int fileId, bool isMain)
: m_context{ context },
  m_diag{ context.getDiag() },
  m_fileId{ fileId },
  m_isMain{ isMain },
  m_scope{ Scope::Root } {
    m_lexer = make_unique<Lexer>(m_context, m_fileId);
    advance();
}

Parser::~Parser() noexcept = default;

/**
 * Module
 *   = StmtList
 *   .
 */
AstModule* Parser::parse() {
    auto* stmts = stmtList();
    return m_context.create<AstModule>(
        m_fileId,
        stmts->range,
        m_isMain,
        stmts);
}

//----------------------------------------
// Statements
//----------------------------------------

/**
 * StmtList
 *   = { Statement }
 *   .
 */
AstStmtList* Parser::stmtList() {
    constexpr auto isNonTerminator = [](const Token& token) {
        switch (token.getKind()) {
        case TokenKind::End:
        case TokenKind::Else:
        case TokenKind::Next:
        case TokenKind::Loop:
        case TokenKind::EndOfFile:
            return false;
        default:
            return true;
        }
    };

    auto start = m_token.range().Start;
    std::vector<AstStmt*> stms;

    while (isNonTerminator(m_token)) {
        stms.emplace_back(statement());
        consume(TokenKind::EndOfStmt);
    }

    return m_context.create<AstStmtList>(
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
AstStmt* Parser::statement() {
    if (auto* decl = declaration()) {
        return decl;
    }

    if (m_scope == Scope::Root) {
        if (m_token.is(TokenKind::Import)) {
            return kwImport();
        }
        if (!m_isMain) {
            m_diag.report(Diag::notAllowedTopLevelStatement, m_token.range());
            std::exit(EXIT_FAILURE);
        }
    }

    switch (m_token.getKind()) {
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

    auto* expr = expression(ExprFlags::UseAssign | ExprFlags::CallWithoutParens);
    return m_context.create<AstExprStmt>(expr->range, expr);
}

/**
 * IMPORT
 *   = "IMPORT" id
 *   .
 */
AstImport* Parser::kwImport() {
    // assume m_token == Import
    assert(m_token.is(TokenKind::Import));
    advance();

    expect(TokenKind::Identifier);
    auto import = m_token.lexeme();
    auto range = m_token.range();
    advance();

    // Imported file
    auto source = m_context.getOptions().getCompilerDir() / "lib" / (import + ".bas").str();
    if (!m_context.import(source.string())) {
        return m_context.create<AstImport>(llvm::SMRange{ range.Start, m_endLoc }, import);
    }
    if (!fs::exists(source)) {
        m_diag.report(Diag::moduleNotFound, range, import);
        std::exit(EXIT_FAILURE);
    }

    // Load import into Source Mgr
    string included;
    auto ID = m_context.getSourceMrg().AddIncludeFile(
        source.string(),
        range.Start,
        included);
    if (ID == ~0U) {
        m_diag.report(Diag::failedToLoadModule, range, source.string());
        std::exit(EXIT_FAILURE);
    }

    // parse the module
    auto* module = Parser(m_context, ID, false).parse();
    return m_context.create<AstImport>(
        llvm::SMRange{ range.Start, m_endLoc },
        import,
        module);
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
AstStmt* Parser::declaration() {
    auto* attribs = attributeList();

    switch (m_token.getKind()) {
    case TokenKind::Var:
        return kwVar(attribs);
    case TokenKind::Declare:
        return kwDeclare(attribs);
    case TokenKind::Function:
    case TokenKind::Sub:
        return kwFunction(attribs);
    case TokenKind::Type:
        return kwType(attribs);
    default:
        break;
    }

    if (attribs != nullptr) {
        m_diag.report(Diag::expectedDeclarationAfterAttribute, m_token.range(), m_token.description());
        std::exit(EXIT_FAILURE);
    }
    return nullptr;
}

//----------------------------------------
// Attributes
//----------------------------------------

/**
 *  AttributeList = [ '[' Attribute { ','  Attribute } ']' ].
 */
AstAttributeList* Parser::attributeList() {
    if (m_token.isNot(TokenKind::BracketOpen)) {
        return nullptr;
    }

    auto start = m_token.range().Start;
    advance();

    std::vector<AstAttribute*> attribs;

    do {
        attribs.emplace_back(attribute());
    } while (accept(TokenKind::Comma));

    consume(TokenKind::BracketClose);

    return m_context.create<AstAttributeList>(
        llvm::SMRange{ start, m_endLoc },
        std::move(attribs));
}

/**
 * Attribute
 *   = IdentExpr [ AttributeArgList ]
 *   .
 */
AstAttribute* Parser::attribute() {
    auto start = m_token.range().Start;

    auto* id = identifier();
    AstExprList* args = nullptr;
    if (m_token.isOneOf(TokenKind::Assign, TokenKind::ParenOpen)) {
        args = attributeArgList();
    }

    return m_context.create<AstAttribute>(
        llvm::SMRange{ start, m_endLoc },
        id,
        args);
}

/**
 * AttributeArgList
 *   = "=" Literal
 *   | "(" [ Literal { "," Literal } ] ")"
 *   .
 */
AstExprList* Parser::attributeArgList() {
    auto start = m_token.range().Start;
    std::vector<AstExpr*> args;

    if (accept(TokenKind::Assign)) {
        args.emplace_back(literal());
    } else if (accept(TokenKind::ParenOpen)) {
        while (!m_token.isOneOf(TokenKind::EndOfFile, TokenKind::ParenClose)) {
            args.emplace_back(literal());
            if (!accept(TokenKind::Comma)) {
                break;
            }
        }
        consume(TokenKind::ParenClose);
    }

    return m_context.create<AstExprList>(
        llvm::SMRange{ start, m_endLoc },
        std::move(args));
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
AstVarDecl* Parser::kwVar(AstAttributeList* attribs) {
    // assume m_token == VAR
    assert(m_token.is(TokenKind::Var));
    auto start = attribs != nullptr ? attribs->range.Start : m_token.range().Start;
    advance();

    expect(TokenKind::Identifier);
    auto id = m_token.getStringValue();
    advance();

    AstTypeExpr* type = nullptr;
    AstExpr* expr = nullptr;

    if (accept(TokenKind::As)) {
        type = typeExpr();
        if (accept(TokenKind::Assign)) {
            expr = expression();
        }
    } else {
        consume(TokenKind::Assign);

        expr = expression();
    }

    return m_context.create<AstVarDecl>(
        llvm::SMRange{ start, m_endLoc },
        id,
        attribs,
        type,
        expr);
}

//----------------------------------------
// DECLARE
//----------------------------------------

/**
 * DECLARE
 *   = "DECLARE" FuncSignature
 *   .
 */
AstFuncDecl* Parser::kwDeclare(AstAttributeList* attribs) {
    // assume m_token == DECLARE
    assert(m_token.is(TokenKind::Declare));
    if (m_scope != Scope::Root) {
        m_diag.report(Diag::unexpectedNestedDeclaration, m_token.range(), m_token.description());
        std::exit(EXIT_FAILURE);
    }
    auto start = attribs != nullptr ? attribs->range.Start : m_token.range().Start;
    advance();

    return funcSignature(start, attribs, false);
}

/**
 * FuncSignature
 *     = "FUNCTION" id [ "(" [ FuncParamList ] ")" ] "AS" TypeExpr
 *     | "SUB" id [ "(" FuncParamList ")" ]
 *     .
 */
AstFuncDecl* Parser::funcSignature(llvm::SMLoc start, AstAttributeList* attribs, bool hasImpl) {
    bool isFunc = accept(TokenKind::Function);
    if (!isFunc) {
        consume(TokenKind::Sub);
    }

    expect(TokenKind::Identifier);
    auto id = m_token.getStringValue();
    advance();

    bool isVariadic = false;
    AstFuncParamList* params = nullptr;
    if (accept(TokenKind::ParenOpen)) {
        params = funcParamList(isVariadic);
        consume(TokenKind::ParenClose);
    }

    AstTypeExpr* ret = nullptr;
    if (isFunc) {
        consume(TokenKind::As);
        ret = typeExpr();
    }

    return m_context.create<AstFuncDecl>(
        llvm::SMRange{ start, m_endLoc },
        id,
        attribs,
        params,
        isVariadic,
        ret,
        hasImpl);
}

/**
 * FuncParamList
 *   = FuncParam { "," FuncParam } [ "," "..." ]
 *   | "..."
 *   .
 */
AstFuncParamList* Parser::funcParamList(bool& isVariadic) {
    auto start = m_token.range().Start;
    std::vector<AstFuncParamDecl*> params;
    while (!m_token.isOneOf(TokenKind::EndOfFile, TokenKind::ParenClose)) {
        if (accept(TokenKind::Ellipsis)) {
            isVariadic = true;
            if (m_token.is(TokenKind::Comma)) {
                m_diag.report(Diag::variadicArgumentNotLast, m_token.range());
                std::exit(EXIT_FAILURE);
            }
            break;
        }
        params.push_back(funcParam());
        if (!accept(TokenKind::Comma)) {
            break;
        }
    }

    return m_context.create<AstFuncParamList>(
        llvm::SMRange{ start, m_endLoc },
        std::move(params));
}

/**
 * FuncParam
 *  = id "AS" TypeExpr
 *  .
 */
AstFuncParamDecl* Parser::funcParam() {
    auto start = m_token.range().Start;

    expect(TokenKind::Identifier);
    auto id = m_token.getStringValue();
    advance();

    consume(TokenKind::As);
    auto* type = typeExpr();

    return m_context.create<AstFuncParamDecl>(
        llvm::SMRange{ start, m_endLoc },
        id,
        nullptr,
        type);
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
AstTypeDecl* Parser::kwType(AstAttributeList* attribs) {
    // assume m_token == TYPE
    assert(m_token.is(TokenKind::Type));
    auto start = m_token.range().Start;
    advance();

    expect(TokenKind::Identifier);
    auto id = m_token.getStringValue();
    advance();

    consume(TokenKind::EndOfStmt);

    auto decls = typeDeclList();

    consume(TokenKind::End);
    consume(TokenKind::Type);

    return m_context.create<AstTypeDecl>(
        llvm::SMRange{ start, m_endLoc },
        id,
        attribs,
        std::move(decls));
}

/**
 * typeDeclList
 *   = { [ AttributeList ] typeMember EoS }
 *   .
 */
std::vector<AstDecl*> Parser::typeDeclList() {
    std::vector<AstDecl*> decls;

    while (true) {
        auto* attribs = attributeList();
        if (attribs != nullptr) {
            expect(TokenKind::Identifier);
        } else if (m_token.isNot(TokenKind::Identifier)) {
            break;
        }

        decls.emplace_back(typeMember(attribs));
        consume(TokenKind::EndOfStmt);
    }

    return decls;
}

/**
 * typeMember
 *   = id "AS" TypeExpr
 *   .
 */
AstDecl* Parser::typeMember(AstAttributeList* attribs) {
    // assume m_token == Identifier
    assert(m_token.is(TokenKind::Identifier));
    auto start = m_token.range().Start;
    auto id = m_token.getStringValue();
    advance();

    consume(TokenKind::As);

    auto* type = typeExpr();

    return m_context.create<AstVarDecl>(
        llvm::SMRange{ start, m_endLoc },
        id,
        attribs,
        type,
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
AstFuncStmt* Parser::kwFunction(AstAttributeList* attribs) {
    if (m_scope != Scope::Root) {
        m_diag.report(Diag::unexpectedNestedDeclaration, m_token.range(), m_token.description());
        std::exit(EXIT_FAILURE);
    }

    auto start = attribs != nullptr ? attribs->range.Start : m_token.range().Start;
    auto* decl = funcSignature(start, attribs, true);
    consume(TokenKind::EndOfStmt);

    RESTORE_ON_EXIT(m_scope);
    m_scope = Scope::Function;

    auto* stmts = stmtList();

    consume(TokenKind::End);

    if (decl->retTypeExpr != nullptr) {
        consume(TokenKind::Function);
    } else {
        consume(TokenKind::Sub);
    }

    return m_context.create<AstFuncStmt>(
        llvm::SMRange{ start, m_endLoc },
        decl,
        stmts);
}

/**
 * RETURN = "RETURN" [ expression ] .
 */
AstStmt* Parser::kwReturn() {
    // assume m_token == RETURN
    assert(m_token.is(TokenKind::Return));
    if (m_scope == Scope::Root && !m_isMain) {
        m_diag.report(Diag::unexpectedReturn, m_token.range());
        std::exit(EXIT_FAILURE);
    }
    auto start = m_token.range().Start;
    advance();

    AstExpr* expr = nullptr;
    if (m_token.isNot(TokenKind::EndOfStmt)) {
        expr = expression();
    }

    return m_context.create<AstReturnStmt>(
        llvm::SMRange{ start, m_endLoc },
        expr);
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
AstIfStmt* Parser::kwIf() {
    // assume m_token == IF
    assert(m_token.is(TokenKind::If));
    auto start = m_token.range().Start;
    advance();

    std::vector<AstIfStmtBlock> blocks;
    blocks.emplace_back(ifBlock());

    if (m_token.is(TokenKind::EndOfStmt)) {
        Token next;
        m_lexer->peek(next);
        if (next.getKind() == TokenKind::Else) {
            advance();
        }
    }

    while (accept(TokenKind::Else)) {
        if (accept(TokenKind::If)) {
            blocks.emplace_back(ifBlock());
        } else {
            blocks.emplace_back(thenBlock({}, nullptr));
        }

        if (m_token.is(TokenKind::EndOfStmt)) {
            Token next;
            m_lexer->peek(next);
            if (next.getKind() == TokenKind::Else) {
                advance();
            }
        }
    }

    if (blocks.back().stmt->kind == AstKind::StmtList) {
        consume(TokenKind::End);
        consume(TokenKind::If);
    }

    return m_context.create<AstIfStmt>(
        llvm::SMRange{ start, m_endLoc },
        std::move(blocks));
}

/**
 * IfBlock
 *   = [ VAR { "," VAR } "," ] Expression "THEN" ThenBlock
 *   .
 */
AstIfStmtBlock Parser::ifBlock() {
    std::vector<AstVarDecl*> decls;
    while (m_token.is(TokenKind::Var)) {
        decls.emplace_back(kwVar(nullptr));
        consume(TokenKind::Comma);
    }

    auto* expr = expression(ExprFlags::CommaAsAnd);
    consume(TokenKind::Then);

    return thenBlock(std::move(decls), expr);
}

/**
 * ThenBlock
 *   =
 *   ( EoS StmtList
 *   | Statement
 *   )
 *   .
 */
[[nodiscard]] AstIfStmtBlock Parser::thenBlock(std::vector<AstVarDecl*> decls, AstExpr* expr) {
    AstStmt* stmt = nullptr;
    if (accept(TokenKind::EndOfStmt)) {
        stmt = stmtList();
    } else {
        stmt = statement();
    }
    return AstIfStmtBlock{ std::move(decls), nullptr, expr, stmt };
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
[[nodiscard]] AstForStmt* Parser::kwFor() {
    // assume m_token == FOR
    assert(m_token.is(TokenKind::For));
    auto start = m_token.range().Start;
    advance();

    std::vector<AstVarDecl*> decls;

    // [ VAR { "," VAR } "," ]
    while (m_token.is(TokenKind::Var)) {
        decls.emplace_back(kwVar(nullptr));
        consume(TokenKind::Comma);
    }

    // id [ "AS" TypeExpr ] "=" Expression
    auto idStart = m_token.range().Start;
    expect(TokenKind::Identifier);
    auto id = m_token.getStringValue();
    advance();

    AstTypeExpr* type = nullptr;
    if (accept(TokenKind::As)) {
        type = typeExpr();
    }

    consume(TokenKind::Assign);

    auto* expr = expression();
    auto* iterator = m_context.create<AstVarDecl>(
        llvm::SMRange{ idStart, m_endLoc },
        id,
        nullptr,
        type,
        expr);

    // "TO" Expression [ "STEP" expression ]
    consume(TokenKind::To);

    auto* limit = expression();
    AstExpr* step = nullptr;
    if (accept(TokenKind::Step)) {
        step = expression();
    }

    // "DO" statement ?
    AstStmt* stmt = nullptr;
    StringRef next;
    if (accept(TokenKind::Do)) {
        stmt = statement();
    } else {
        consume(TokenKind::EndOfStmt);

        stmt = stmtList();

        consume(TokenKind::Next);

        if (m_token.is(TokenKind::Identifier)) {
            next = m_token.getStringValue();
            advance();
        }
    }

    return m_context.create<AstForStmt>(
        llvm::SMRange{ start, m_endLoc },
        std::move(decls),
        iterator,
        limit,
        step,
        stmt,
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
[[nodiscard]] AstDoLoopStmt* Parser::kwDo() {
    // assume m_token == DO
    assert(m_token.is(TokenKind::Do));
    auto start = m_token.range().Start;
    advance();

    auto condition = AstDoLoopStmt::Condition::None;
    AstStmt* stmt = nullptr;
    AstExpr* expr = nullptr;
    std::vector<AstVarDecl*> decls;

    // [ VAR { "," VAR } ]
    auto acceptComma = false;
    while (m_token.is(TokenKind::Var) || (acceptComma && accept(TokenKind::Comma))) {
        acceptComma = true;
        decls.emplace_back(kwVar(nullptr));
    }

    // ( EoS StmtList "LOOP" [ Condition ]
    if (accept(TokenKind::EndOfStmt)) {
        stmt = stmtList();

        consume(TokenKind::Loop);

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

            consume(TokenKind::Loop);
        }
        // "DO" Statement
        else {
            consume(TokenKind::Do);

            stmt = statement();
        }
    }

    return m_context.create<AstDoLoopStmt>(
        llvm::SMRange{ start, m_endLoc },
        std::move(decls),
        condition,
        expr,
        stmt);
}

//----------------------------------------
// Branching
//----------------------------------------

/**
 * CONTINUE
 *   = "CONTINUE" { "FOR" }
 *   .
 */
AstContinuationStmt* Parser::kwContinue() {
    // assume m_token == CONTINUE
    assert(m_token.is(TokenKind::Continue));
    auto start = m_token.range().Start;
    advance();

    std::vector<ControlFlowStatement> returnControl;

    while (true) {
        switch (m_token.getKind()) {
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

    return m_context.create<AstContinuationStmt>(
        llvm::SMRange{ start, m_endLoc },
        AstContinuationStmt::Action::Continue,
        std::move(returnControl));
}

/**
 * EXIT
 *   = "EXIT" { "FOR" }
 *   .
 */
AstContinuationStmt* Parser::kwExit() {
    // assume m_token == EXIT
    assert(m_token.is(TokenKind::Exit));
    auto start = m_token.range().Start;
    advance();

    std::vector<ControlFlowStatement> returnControl;

    while (true) {
        switch (m_token.getKind()) {
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

    return m_context.create<AstContinuationStmt>(
        llvm::SMRange{ start, m_endLoc },
        AstContinuationStmt::Action::Exit,
        std::move(returnControl));
}

//----------------------------------------
// Types
//----------------------------------------

/**
 * TypeExpr = ( identExpr | Any ) { "PTR" } .
 */
AstTypeExpr* Parser::typeExpr() {
    auto start = m_token.range().Start;
    auto kind = m_token.getKind();

    AstIdentExpr* ident = nullptr;
    if (m_token.is(TokenKind::Any) || m_token.isTypeKeyword()) {
        advance();
    } else {
        ident = identifier();
    }

    auto deref = 0;
    while (accept(TokenKind::Ptr)) {
        deref++;
    }

    return m_context.create<AstTypeExpr>(
        llvm::SMRange{ start, m_endLoc },
        ident,
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
AstExpr* Parser::expression(ExprFlags flags) {
    RESTORE_ON_EXIT(m_exprFlags);
    m_exprFlags = flags;

    auto* expr = factor();

    if ((flags & ExprFlags::UseAssign) == 0) {
        replace(TokenKind::Assign, TokenKind::Equal);
    }
    if ((flags & ExprFlags::CommaAsAnd) != 0) {
        replace(TokenKind::Comma, TokenKind::CommaAnd);
    }
    if (m_token.isOperator()) {
        expr = expression(expr, 1);
    }

    if ((m_exprFlags & ExprFlags::CallWithoutParens) != 0 && m_token.isNot(TokenKind::EndOfStmt)) {
        if (m_token.is(TokenKind::Identifier) || m_token.isLiteral() || m_token.isUnary()) {
            auto start = expr->range.Start;
            auto* args = expressionList();

            return m_context.create<AstCallExpr>(
                llvm::SMRange{ start, m_endLoc },
                expr,
                args);
        }
    }

    return expr;
}

/**
 * factor = primary { <Right Unary Op> | "AS" TypeExpr } .
 */
AstExpr* Parser::factor() {
    auto start = m_token.range().Start;
    auto* expr = primary();

    while (true) {
        // <Right Unary Op>
        if (m_token.isUnary() && m_token.isRightToLeft()) {
            auto kind = m_token.getKind();
            advance();

            expr = unary({ start, m_endLoc }, kind, expr);
            continue;
        }

        // "AS" TypeExpr
        if (accept(TokenKind::As)) {
            auto* type = typeExpr();
            auto* cast = m_context.create<AstCastExpr>(
                llvm::SMRange{ start, m_endLoc },
                expr,
                type,
                false);
            expr = cast;
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
AstExpr* Parser::primary() {
    if (m_token.isLiteral()) {
        return literal();
    }

    // TODO callExpr should be resolved in the expression
    if (m_token.is(TokenKind::Identifier)) {
        Token next;
        m_lexer->peek(next);
        if (next.is(TokenKind::ParenOpen)) {
            return callExpr();
        }
        return identifier();
    }

    if (accept(TokenKind::ParenOpen)) {
        auto* expr = expression();
        consume(TokenKind::ParenClose);
        return expr;
    }

    if (m_token.is(TokenKind::If)) {
        return ifExpr();
    }

    replace(TokenKind::Minus, TokenKind::Negate);
    replace(TokenKind::Multiply, TokenKind::Dereference);
    if (m_token.isUnary() && m_token.isLeftToRight()) {
        auto start = m_token.range().Start;
        auto prec = m_token.getPrecedence();
        auto kind = m_token.getKind();
        advance();

        if ((m_exprFlags & ExprFlags::UseAssign) == 0) {
            replace(TokenKind::Assign, TokenKind::Equal);
        }
        auto* expr = expression(factor(), prec);

        return unary({ start, m_endLoc }, kind, expr);
    }

    m_diag.report(Diag::expectedExpression, m_token.range(), m_token.description());
    std::exit(EXIT_FAILURE);
}

AstExpr* Parser::unary(llvm::SMRange range, TokenKind op, AstExpr* expr) {
    switch (op) {
    case TokenKind::Dereference:
        return m_context.create<AstDereference>(range, expr);
    case TokenKind::AddressOf:
        return m_context.create<AstAddressOf>(range, expr);
    default:
        return m_context.create<AstUnaryExpr>(range, op, expr);
    }
}

AstExpr* Parser::binary(llvm::SMRange range, TokenKind op, AstExpr* lhs, AstExpr* rhs) {
    switch (op) {
    case TokenKind::CommaAnd:
        return m_context.create<AstBinaryExpr>(range, TokenKind::LogicalAnd, lhs, rhs);
    case TokenKind::Assign:
        return m_context.create<AstAssignExpr>(range, lhs, rhs);
    case TokenKind::MemberAccess:
        return m_context.create<AstMemberAccess>(range, lhs, rhs);
    default:
        return m_context.create<AstBinaryExpr>(range, op, lhs, rhs);
    }
}

/**
 * Recursievly climb operator precedence
 * https://en.wikipedia.org/wiki/Operator-precedence_parser#Precedence_climbing_method
 */
AstExpr* Parser::expression(AstExpr* lhs, int precedence) {
    while (m_token.getPrecedence() >= precedence) {
        auto current = m_token.getPrecedence();
        auto kind = m_token.getKind();
        advance();

        auto* rhs = factor();
        if ((m_exprFlags & ExprFlags::UseAssign) == 0) {
            replace(TokenKind::Assign, TokenKind::Equal);
        }
        if ((m_exprFlags & ExprFlags::CommaAsAnd) != 0) {
            replace(TokenKind::Comma, TokenKind::CommaAnd);
        }

        while (m_token.getPrecedence() > current || (m_token.isRightToLeft() && m_token.getPrecedence() == current)) {
            rhs = expression(rhs, m_token.getPrecedence());
        }

        auto start = lhs->range.Start;
        lhs = binary({ start, m_endLoc }, kind, lhs, rhs);
    }
    return lhs;
}

/**
 * IdentExpr
 *   = id
 *   .
 */
AstIdentExpr* Parser::identifier() {
    auto start = m_token.range().Start;
    expect(TokenKind::Identifier);
    auto name = m_token.getStringValue();
    advance();

    return m_context.create<AstIdentExpr>(
        llvm::SMRange{ start, m_endLoc },
        name);
}

/**
 * callExpr = identifier "(" argList ")" .
 */
AstCallExpr* Parser::callExpr() {
    auto start = m_token.range().Start;
    auto* id = identifier();

    consume(TokenKind::ParenOpen);
    auto* args = expressionList();
    consume(TokenKind::ParenClose);

    return m_context.create<AstCallExpr>(
        llvm::SMRange{ start, m_endLoc },
        id,
        args);
}

/**
 * IfExpr = "IF" expr "THEN" expr "ELSE" expr .
 */
AstIfExpr* Parser::ifExpr() {
    // assume m_token == IF
    assert(m_token.is(TokenKind::If));
    auto start = m_token.range().Start;
    advance();

    auto* expr = expression(ExprFlags::CommaAsAnd);

    consume(TokenKind::Then);
    auto* trueExpr = expression();

    consume(TokenKind::Else);
    auto* falseExpr = expression();

    return m_context.create<AstIfExpr>(
        llvm::SMRange{ start, m_endLoc },
        expr,
        trueExpr,
        falseExpr);
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
AstLiteralExpr* Parser::literal() {
    auto value = m_token.getValue();
    advance();

    return m_context.create<AstLiteralExpr>(
        m_token.range(),
        value);
}

/**
 * Parse comma separated list of expressionds
 */
AstExprList* Parser::expressionList() {
    auto start = m_token.range().Start;
    std::vector<AstExpr*> exprs;

    while (!m_token.isOneOf(TokenKind::EndOfFile, TokenKind::ParenClose, TokenKind::EndOfStmt)) {
        exprs.emplace_back(expression());
        if (!accept(TokenKind::Comma)) {
            break;
        }
    }

    return m_context.create<AstExprList>(
        llvm::SMRange{ start, m_endLoc },
        std::move(exprs));
}

//----------------------------------------
// Helpers
//----------------------------------------

void Parser::replace(TokenKind what, TokenKind with) noexcept {
    if (m_token.is(what)) {
        m_token.setKind(with);
    }
}

bool Parser::expect(TokenKind kind) noexcept {
    if (m_token.is(kind)) {
        return true;
    }

    m_context.getDiag().report(
        Diag::unexpectedToken,
        m_token.range(),
        Token::description(kind),
        m_token.description());

    std::exit(EXIT_FAILURE);
}

void Parser::advance() {
    m_endLoc = m_token.range().End;
    m_lexer->next(m_token);
}
