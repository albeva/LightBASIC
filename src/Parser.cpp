//
//  Parser.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "Parser.h"
#include "Context.h"
#include "SourceFile.h"
#include "Lexer.h"
#include "Ast.h"
#include "Token.h"
using namespace lbc;


#define expect(_tok) if (!accept(_tok)) THROW_EXCEPTION("Unexpected token. Found " + m_token->lexeme() + ". Expected " + Token::getTokenName(_tok));

/**
 * Create the parser object
 */
Parser::Parser(const shared_ptr<Context> & ctx)
: m_ctx(ctx), m_token(nullptr), m_next(nullptr), m_lexer(nullptr), m_expectAssign(false)
{
}


/**
 * Parse the input and return the Ast tree
 * Program = { Declaration }
 */
AstProgram * Parser::parse(const shared_ptr<Source> & source)
{
    m_token = nullptr;
    m_next = nullptr;
    m_expectAssign = false;
    
    // init the lexer
    m_lexer = new Lexer(source);
    move(); move();
    
    // resulting ast node
    auto ast = new AstProgram(source->getName());
    
    // { DeclList }
    while (!match(TokenType::EndOfFile)) {
        ast->decls.push_back(unique_ptr<AstDeclaration>(declaration()));
        expect(TokenType::EndOfLine);
    }
    
    // clean up
    delete m_lexer;
    m_lexer = nullptr;
    
    // done
    return ast;
}


/**
 * Declaration  = [ "[" Attributes "]" ]
 *              ( VariableDecl
 *              | FunctionDecl
 *              | FunctionImpl
 *              )
 */
AstDeclaration * Parser::declaration()
{
    AstAttributeList * attribs = nullptr;
    AstDeclaration * decl = nullptr;
    
    // [ ... ]
    if (accept(TokenType::BracketOpen)) {
        attribs = attributesList();
        expect(TokenType::BracketClose);
    }

    switch (m_token->type()) {
        case TokenType::Dim:
        case TokenType::Var:
            decl = variableDecl();
            break;
        case TokenType::Declare:
            decl = functionDecl();
            break;
        case TokenType::Function:
        case TokenType::Sub:
            decl = functionStmt();
            break;
        default:
            THROW_EXCEPTION(string("Invalid input. Expected declaration. Found: ") + m_token->name());
            break;
    }
    
    // add attribs
    if (decl && attribs) {
        decl->attribs.reset(attribs);
    }
    
    // done
    return decl;
}


/**
 * StatementList = { Statement }
 */
AstStmtList * Parser::statementList()
{
    auto ast = new AstStmtList();
    
    // { Statement }
    while(!match(TokenType::EndOfFile)) {
        // end of compound statements
        switch(m_token->type()) {
            case TokenType::End:
            case TokenType::Else:
            case TokenType::Next:
                return ast;
            default: {
                auto stmt = statement();
                if (stmt) ast->stmts.push_back(unique_ptr<AstStatement>(stmt));
                expect(TokenType::EndOfLine);
            }
        }
    }
    
    return ast;
}


/**
 * Statement    = VariableDecl
 *              | AssignStmt
 *              | FuncCallExpr
 *              | ReturnStmt
 *              | IfStmt
 *              | ForStmt
 */
AstStatement * Parser::statement()
{
    AstStatement * stmt = nullptr;
    switch (m_token->type()) {
        case TokenType::Dim:
        case TokenType::Var:
            stmt = variableDecl();
            break;
        case TokenType::Return:
            stmt = returnStmt();
            break;
        case TokenType::Identifier:
            if (m_next->type() == TokenType::Assign) {
                stmt = assignStmt();
            } else /* if (m_next->type() == TokenType::ParenOpen) */ {
                stmt = callStmt();
            }
            break;
        // NOTE probably very wrong. at the moment assume
        // that line starting with * is assignment to dereferenced
        // pointer
        // *ip = 10
        case TokenType::Dereference:
            stmt = assignStmt();
            break;
        case TokenType::If:
            stmt = ifStmt();
            break;
        case TokenType::For:
            stmt = forStmt();
            break;
        default:
            THROW_EXCEPTION(string("Invalid input. Expected statement. Found: ") + m_token->name());
            break;
    }
    
    return stmt;
}


/**
 * AssignStmt = Expression "=" Expression
 */
AstAssignStmt * Parser::assignStmt()
{
    // left
    m_expectAssign = true;
    auto left = expression();
    m_expectAssign = false;
    
    // =
    expect(TokenType::Assign);
    
    // right
    auto right = expression();
    
    // done
    return new AstAssignStmt(left, right);
}


/**
 * callStmt = id ( "(" FuncArgList ")" | FuncArgList ) 
 *
 * This doesn't conform to ebnf. Basically since parens are optional
 * for callstmt then need to distinguish between the following:
 * foo
 * foo(1, 2)
 * foo 1, 2
 * foo (1), 2
 * foo 1, (2)
 * foo (1), (2)
 */
AstCallStmt * Parser::callStmt()
{
    // id
    auto id = identifier();
    
    // "("
    bool paren = accept(TokenType::ParenOpen);
    
    // ")"
    if ((paren && accept(TokenType::ParenClose)) || (!paren && match(TokenType::EndOfLine))) {
        return new AstCallStmt(new AstCallExpr(id, nullptr));
    }
    
    auto args = new AstFuncArgList();
    
    // get 1st argument
    args->args.push_back(unique_ptr<AstExpression>(expression()));
    
    // is it arg list enclosed in ( and )
    // or are the braces part of the expression?
    if (paren && accept(TokenType::ParenClose)) {
        paren = false;
    }
    
    // rest of the arguments
    while (accept(TokenType::Comma)) {
        args->args.push_back(unique_ptr<AstExpression>(expression()));
    }
    
    // expect a closing ) ?
    if (paren) {
        expect(TokenType::ParenClose);
    }
    
    // done
    return new AstCallStmt(new AstCallExpr(id, args));
}


/**
 * ifStmt   = "IF" Expression "THEN"
 *            StmtList
 *            ( "ELSE" ( ifStmt | StmtList "END" "IF" )
 *            | "END" "IF"
 *            )
 */
AstIfStmt * Parser::ifStmt()
{
    // if
    expect(TokenType::If);
    
    // expression
    auto expr = expression();
    
    // then
    expect(TokenType::Then);
    
    // End Of Line
    expect(TokenType::EndOfLine);
    
    // stmtlist if true
    auto trueBlock = statementList();
    AstStatement * falseBlock = nullptr;
    
    // else ?
    bool expectEndIf = true;
    if (accept(TokenType::Else)) {
        // else if ?
        if (match(TokenType::If)) {
            falseBlock = ifStmt();
            expectEndIf = false;
        } else {
            expect(TokenType::EndOfLine);
            falseBlock = statementList();
        }
    }
    
    // if was Else IF
    // then recursive ifStmt will parse the End If
    if (expectEndIf) {
        expect(TokenType::End);
        expect(TokenType::If);
    }
    
    // done
    return new AstIfStmt(expr, trueBlock, falseBlock);
}


/**
 * ForStmt  = "FOR" ( id [ "AS" TypeExpr ]
 *                  | "Var" id
 *                  | Expression
 *                  ) "=" Expression "TO" Expression [ "STEP" Expression ] \n
 *                StmtList
 *            "NEXT"
 */
AstForStmt * Parser::forStmt()
{
    // "FOR"
    expect(TokenType::For);
    
    // ( VariableDecl | AssignStmt )
    AstStatement * stmt = nullptr;
    if (match(TokenType::Var) || (match(TokenType::Identifier) && m_next->type() == TokenType::As)) {
        auto dim = variableDecl(m_next->type() == TokenType::As);
        if (!dim->expr) {
            THROW_EXCEPTION("Expect expression");
        }
        stmt = dim;
    } else {
        stmt = assignStmt();
    }
    
    // "TO" expression
    expect(TokenType::To);
    auto end = expression();
    
    // ["STEP" expression]
    auto step = accept(TokenType::Step) ? expression() : nullptr;
    
    // end of line
    expect(TokenType::EndOfLine);
    
    // StmtList
    auto block = statementList();
    
    // "NEXT"
    expect(TokenType::Next);
    
    // done
    return new AstForStmt(stmt, end, step, block);
}


/**
 * ReturnStmt = "RETURN" Expression
 */
AstReturnStmt * Parser::returnStmt()
{
    expect(TokenType::Return);
    return new AstReturnStmt(match(TokenType::EndOfLine) ? nullptr : expression());
}


/**
 * Expression = IntegerLiteral
 *            | FloatingPointLiteral
 *            | StringLiteral
 *            | CallExpr
 *            | id
 *            | AddressOf
 *            | Dereference
 *            | True
 *            | False
 *            | Null
 *            | "(" Expression ")"
 *            | Comparison
 *            | Mod
 *
 * for the moment use hackish way to add
 * a binary expression.
 * later will use an expression parser
 * that is not easily expressed in ebnf anyways
 */
AstExpression * Parser::expression()
{
    AstExpression * expr = nullptr;
    
    // create scope
    {
        SCOPED_GUARD(m_expectAssign);
        switch (m_token->type()) {
            // IntegerLiteral
            // FloatingPointLiteral
            // StringLiteral
            case TokenType::IntegerLiteral:
            case TokenType::StringLiteral:
            case TokenType::FloatingPointLiteral:
                expr = new AstLiteralExpr(m_token);
                move();
                break;
            // true, false
            case TokenType::True:
            case TokenType::False:
                expr = new AstLiteralExpr(m_token);
                move();
                break;
            // null
            case TokenType::Null:
                expr = new AstLiteralExpr(m_token);
                move();
                break;
            // CallExpr
            // id
            case TokenType::Identifier:
                if (m_next->type() == TokenType::ParenOpen) {
                    m_expectAssign = false;
                    expr = callExpr();
                    break;
                } else {
                    expr = identifier();
                    break;
                }
            // AddressOf
            case TokenType::AddressOf:
                move();
                expr = new AstAddressOfExpr(identifier());
                break;
            // Dereference
            case TokenType::Dereference:
                move();
                expr = new AstDereferenceExpr(expression());
                break;
            // ( Expression )
            case TokenType::ParenOpen:
                move();
                expr = expression();
                expect(TokenType::ParenClose);
                break;
            default:
                THROW_EXCEPTION(string("Invalid input. Expected expression. Found: ") + m_token->name());
        }
    }
    
    // =
    if (!m_expectAssign) {
        switch (m_token->type()) {
            case TokenType::Assign:
                m_token->type(TokenType::Equal);
                // fall through!
            case TokenType::Modulus:
            case TokenType::NotEqual:
            case TokenType::GreaterThan:
            case TokenType::GreaterThanEqual:
            case TokenType::LessThan:
            case TokenType::LessThanEqual:
            {
                auto op = m_token;
                move();
                expr = new AstBinaryExpr(op, expr, expression());
                break;
            }
            default:
                break;
        }
    }
    
    return expr;
}


/**
 * CallExpr = id "(" [ funcArgList ] ")"
 */
AstCallExpr * Parser::callExpr()
{
    // id
    auto id = identifier();
    AstFuncArgList * args = nullptr;
    
    // "("
    expect(TokenType::ParenOpen);
    
    // [ funcArgList ]
    if (!accept(TokenType::ParenClose)) {
        args = funcArgList();
        // ")"
        expect(TokenType::ParenClose);
    }
    
    return new AstCallExpr(id, args);
}


/**
 * funcArgList = Expression { "," Expression }
 */
AstFuncArgList * Parser::funcArgList()
{
    auto ast = new AstFuncArgList();
    
    do {
        auto arg = expression();
        if (arg) ast->args.push_back(unique_ptr<AstExpression>(arg));
    } while (accept(TokenType::Comma));
    
    return ast;
}


/**
 * FunctionImpl = FuncSignature
 *              StatementList
 *              "END" "FUNCTION"
 */
AstFunctionStmt * Parser::functionStmt()
{
    // FuncSignature
    auto sig = funcSignature();
    // no vararg support within lbc for now
    if (sig->vararg) {
        THROW_EXCEPTION("Variable arguments not supported");
    }
    expect(TokenType::EndOfLine);
    
    // StatementList
    auto stmts = statementList();
    
    // END FUNCTION or SUB
    expect(TokenType::End);
    if (sig->typeExpr) {
        expect(TokenType::Function);
    } else {
        expect(TokenType::Sub);
    }
    
    // done
    return new AstFunctionStmt(sig, stmts);
}


/**
 * FunctionDecl = "DECLARE" FuncSignature
 */
AstFunctionDecl * Parser::functionDecl()
{
    expect(TokenType::Declare);
    return new AstFunctionDecl(funcSignature());
}


/**
 * FuncSignature = "FUNCTION" id "(" [ FuncParamList ] ")" "AS" TypeExpr
 */
AstFuncSignature * Parser::funcSignature()
{
    // sub or function
    bool isSub = accept(TokenType::Sub);
    if (!isSub) {
        expect(TokenType::Function);
    }
    
    // id
    auto id = identifier();
    bool vararg = false;
    
    // args
    AstFuncParamList * params = nullptr;
    
    // make ( and ) optional
    if (accept(TokenType::ParenOpen)) {
        // [ FuncArgumentList ]
        if (!accept(TokenType::ParenClose)) {
            
            params = funcParamList();
            
            // ellipsis
            if (accept(TokenType::Ellipsis)) {
                vararg = true;
            }
            
            // ")"
            expect(TokenType::ParenClose);
        }
    }
    
    // return type
    AstTypeExpr * type = nullptr;
    
    // if is a function then expect a type declaration
    if (!isSub) {
        // AS
        expect(TokenType::As);
        
        // type
        type = typeExpr();   
    }
    
    // done
    return new AstFuncSignature(id, params, type, vararg);
}


/**
 * FuncParamList = FuncParam { "," FuncParam }
 */
AstFuncParamList * Parser::funcParamList()
{
    auto ast = new AstFuncParamList();
    do {
        if (match(TokenType::Ellipsis)) break;
        auto param = funcParam();
        if (param != nullptr) ast->params.push_back(unique_ptr<AstFuncParam>(param));
    } while (accept(TokenType::Comma));
    
    return ast;
}


/**
 *  FuncParam = id "AS" TypeExpr
 */
AstFuncParam * Parser::funcParam()
{
    // id
    auto id = identifier();
    // AS
    expect(TokenType::As);
    // type
    auto type = typeExpr();
    
    // done
    return new AstFuncParam(id, type);
}


/**
 * VariableDecl = "VAR" id "=" Expression
 *              | "DIM" id "AS" TypeExpr [ "=" Expression ]
 */
AstVarDecl * Parser::variableDecl(bool skipKw)
{
    // VAR ?
    if (accept(TokenType::Var)) {
        // id
        auto id = identifier();
        // "="
        expect(TokenType::Assign);
        // Expression
        auto expr = expression();
        // done
        return new AstVarDecl(id, nullptr, expr);
    }
    
    if (!skipKw) {
        // DIM
        expect(TokenType::Dim);
    }
    // id
    auto id = identifier();
    // AS
    expect(TokenType::As);
    // TypeExpr
    auto t = typeExpr();
    // [ "=" Expression ]
    auto expr = accept(TokenType::Assign) ? expression() : nullptr;
    // done
    return new AstVarDecl(id, t, expr);
}


/**
 * TypeExpr = ("INTEGER" | "BYTE" | "ANY" "PTR") { "PTR" }
 */
AstTypeExpr * Parser::typeExpr()
{
    AstTypeExpr * ast = nullptr;
    Token * tmp = m_token;
    #define EXPECT_TYPE(ID, ...) accept(TokenType::ID) ||
    if (KEYWORD_TYPES(EXPECT_TYPE) false) {
        int deref = 0;
        while (accept(TokenType::Ptr)) deref++;
        ast = new AstTypeExpr(tmp, deref);
    } else if (accept(TokenType::Any)) {
        expect(TokenType::Ptr);
        int deref = 1;
        while (accept(TokenType::Ptr)) deref++;
        ast = new AstTypeExpr(tmp, deref);        
    } else {
        THROW_EXCEPTION("Expected type");
    }
    return ast;
}


/**
 * Attributes = Attribute { ","  Attribute }
 */
AstAttributeList * Parser::attributesList()
{
    auto list = new AstAttributeList();
    
    do {
        auto attrib = attribute();
        if (attrib != nullptr) {
            list->attribs.push_back(unique_ptr<AstAttribute>(attrib));
        }
    } while (accept(TokenType::Comma));
    
    return list;
}


/**
 * Attribute    = id [
 *                  ( "=" AttribParam
 *                  | "(" [ AttribParamList ] ")"
 *                  )
 *              ]
 */
AstAttribute * Parser::attribute()
{
    // id
    AstAttribute * attr = new AstAttribute(identifier());
    
    // "(" AttribParam { "," AttribParam } ")"
    if (accept(TokenType::ParenOpen)) {
        if (!accept(TokenType::ParenClose)) {
            attr->params.reset(attribParamList());
            expect(TokenType::ParenClose);
        }
    }
    // "=" AttribParam
    else if (accept(TokenType::Assign)) {
        auto params = new AstAttribParamList();
        params->params.push_back(unique_ptr<AstLiteralExpr>(attribParam()));
        attr->params.reset(params);
    }
    
    return attr;
}


/**
 * AttribParamList = AttribParam { "," AttribParam }
 */
AstAttribParamList * Parser::attribParamList()
{
    auto params = new AstAttribParamList();
    do {
        auto param = attribParam();
        if (param) {
            params->params.push_back(unique_ptr<AstLiteralExpr>(param));
        }
    } while(accept(TokenType::Comma));
    return params;
}


/**
 * AttribParam  = IntegerLiteral
 *              | StringLiteral
 */
AstLiteralExpr * Parser::attribParam()
{
    auto tmp = m_token;
    if (accept(TokenType::StringLiteral) || accept(TokenType::IntegerLiteral)) {
        return new AstLiteralExpr(tmp);
    }
    expect(TokenType::StringLiteral);
    return nullptr;
}


/**
 * expect identifier
 */
AstIdentExpr * Parser::identifier()
{
    Token * tmp = m_token;
    expect(TokenType::Identifier);
    return new AstIdentExpr(tmp);
}


/**
 * match the token
 */
bool Parser::match(TokenType type) const
{
    return m_token->type() == type;
}


/**
 * Accept token of a type and move if matches
 */
bool Parser::accept(TokenType type)
{
    if (match(type)) {
        move();
        return true;
    }
    return false;
}


///**
// * Expect token. Throw an error if doesn't match
// */
//void Parser::expect(TokenType type)
//{
//    if (!accept(type)) {
//        THROW_EXCEPTION("Unexpected token. Found " + m_token->lexeme() + ". Expected " + Token::getTokenName(type));
//    }
//}


/**
 * mobe to the next token
 */
void Parser::move()
{
    m_token = m_next;
    m_next = m_lexer->next();
}
