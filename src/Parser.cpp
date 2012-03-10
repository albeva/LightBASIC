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

/**
 * Create the parser object
 */
Parser::Parser(const shared_ptr<Context> & ctx)
: m_ctx(ctx), m_token(nullptr), m_next(nullptr), m_lexer(nullptr)
{
}


/**
 * Parse the input and return the Ast tree
 * Program = { Declaration }
 */
AstProgram * Parser::parse(const shared_ptr<Source> & source)
{
    // init the lexer
    m_lexer = new Lexer(source);
    move(); move();
    
    // resulting ast node
    auto ast = new AstProgram(source->getName());
    
    // { DeclList }
    while (!match(TokenType::EndOfFile)) {
        ast->decls.push_back(declaration());
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
            decl = variableDecl();
            break;
        case TokenType::Declare:
            decl = functionDecl();
            break;
        case TokenType::Function:
            decl = functionStmt();
            break;
        default:
            throw Exception(string("Invalid input. Expected declaration. Found: ") + m_token->name());
            break;
    }
    
    // add attribs
    if (decl && attribs) {
        decl->attribs = attribs;
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
    while(!match(TokenType::End) && !match(TokenType::EndOfFile)) {
        auto stmt = statement();
        if (stmt) ast->stmts.push_back(stmt);
        expect(TokenType::EndOfLine);
    }
    
    return ast;
}


/**
 * Statement    = VariableDecl
 *              | AssignStmt
 *              | FuncCallExpr
 *              | ReturnStmt
 */
AstStatement * Parser::statement()
{
    AstStatement * stmt = nullptr;
    switch (m_token->type()) {
        case TokenType::Dim:
            stmt = variableDecl();
            break;
        case TokenType::Return:
            stmt = returnStmt();
            break;
        case TokenType::Identifier:
            if (m_next->type() == TokenType::Assign) {
                stmt = assignStmt();
                break;
            } else if (m_next->type() == TokenType::ParenOpen) {
                stmt = callStmt();
                break;
            }
        default:
            throw Exception(string("Invalid input. Expected statement. Found: ") + m_token->name());
            break;
    }
    
    return stmt;
}


/**
 * AssignStmt = id "=" Expression
 */
AstAssignStmt * Parser::assignStmt()
{
    // id
    auto id = identifier();
    // =
    expect(TokenType::Assign);
    // expression
    auto expr = expression();
    
    // done
    return new AstAssignStmt(id, expr);
}


/**
 * callStmt = callExpr
 */
AstCallStmt * Parser::callStmt()
{
    return new AstCallStmt(callExpr());
}


/**
 * ReturnStmt = "RETURN" Expression
 */
AstReturnStmt * Parser::returnStmt()
{
    expect(TokenType::Return);
    return new AstReturnStmt(expression());
}


/**
 * Expression   = IntegerLiteral
 *              | StringLiteral
 *              | FuncCallExpr
 *              | id
 */
AstExpression * Parser::expression()
{
    AstExpression * expr = nullptr;
    
    switch (m_token->type()) {
        case TokenType::NumericLiteral:
        case TokenType::StringLiteral:
            expr = new AstLiteralExpr(m_token);
            move();
            break;
        case TokenType::Identifier:
            if (m_next->type() == TokenType::ParenOpen) {
                expr = callExpr();
                break;
            } else {
                expr = identifier();
                break;
            }
        default:
            throw Exception(string("Invalid input. Expected expression. Found: ") + m_token->name());
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
        if (arg) ast->args.push_back(arg);
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
        throw Exception("Variable arguments not supported");
    }
    
    expect(TokenType::EndOfLine);
    // StatementList
    auto stmts = statementList();
    // END FUNCTION
    expect(TokenType::End);
    expect(TokenType::Function);
    
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
    // FUNCTION
    expect(TokenType::Function);
    
    // id
    auto id = identifier();
    bool vararg = false;
    
    // args
    AstFuncParamList * params = nullptr;
    
    // "("
    expect(TokenType::ParenOpen);
    
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
    
    // AS
    expect(TokenType::As);
    
    // type
    auto type = typeExpr();
    
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
        if (param != nullptr) ast->params.push_back(param);
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
 * VariableDecl = "DIM" id "AS" TypeExpr
 */
AstVarDecl * Parser::variableDecl()
{
    // DIM
    expect(TokenType::Dim);
    // id
    auto id = identifier();
    // AS
    expect(TokenType::As);
    // TypeExpr
    auto t = typeExpr();
    // done
    return new AstVarDecl(id, t);
}


/**
 * TypeExpr = ("INTEGER" | "BYTE") { "PTR" }
 */
AstTypeExpr * Parser::typeExpr()
{
    AstTypeExpr * ast = nullptr;
    Token * tmp = m_token;
#define EXPECT_TYPE(ID, ...) accept(TokenType::ID) ||
//    if (accept(TokenType::Integer) || accept(TokenType::Byte)) {
    if (ALL_TYPES(EXPECT_TYPE) false) {
        int deref = 0;
        while (accept(TokenType::Ptr)) deref++;
        ast = new AstTypeExpr(tmp, deref);
    } else {
        throw Exception("Expected type");
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
            list->attribs.push_back(attrib);
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
            attr->params = attribParamList();
            expect(TokenType::ParenClose);
        }
    }
    // "=" AttribParam
    else if (accept(TokenType::Assign)) {
        auto params = new AstAttribParamList();
        params->params.push_back(attribParam());
        attr->params = params;
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
            params->params.push_back(param);
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
    if (accept(TokenType::StringLiteral) || accept(TokenType::NumericLiteral)) {
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


/**
 * Expect token. Throw an error if doesn't match
 */
void Parser::expect(TokenType type)
{
    if (!accept(type)) {
        throw Exception("Unexpected token. Found " + m_token->lexeme() + ". Expected " + Token::getTokenName(type));
    }
}


/**
 * mobe to the next token
 */
void Parser::move()
{
    m_token = m_next;
    m_next = m_lexer->next();
}
