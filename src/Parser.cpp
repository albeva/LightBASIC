//
//  Parser.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "ParserShared.h"
using namespace lbc;

/**
 * Create the parser object
 */
Parser::Parser(Context & ctx)
:   m_ctx(ctx),
    m_lexer(nullptr)
{
}


/**
 * Parse the input and return the Ast tree
 * Program = { Declaration }
 */
AstProgram * Parser::parse(const std::string & file)
{
    // reset the state
    m_token         = nullptr;
    m_next          = nullptr;
    m_expectAssign  = false;
    m_hasError      = false;
    
    // init the lexer
    auto & srcMgr = m_ctx.sourceMrg();
    std::string s = "";
    auto ID = srcMgr.AddIncludeFile(file, llvm::SMLoc(), s);
    if (ID == ~0U) {
        // TODO: Raise an error
        return nullptr;
    }
    
    m_lexer = new Lexer(srcMgr.getMemoryBuffer(ID));
    move(); move();
    
    // resulting ast node
    auto ast = new AstProgram(file);
    
    // { DeclList }
    while (!match(TokenType::EndOfFile)) {
        ast->decls.push_back(declaration());
        if (hasError()) return nullptr;
        EXPECT(TokenType::EndOfLine);
    }
    
    // clean up
    delete m_lexer;
    m_lexer = nullptr;
    
    // done
    return ast;
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
bool Parser::expect(TokenType type)
{
    if (!accept(type)) {
            // TODO raise error message. Expect XXX found ZZZ
        m_hasError = true;
        return false;
    }
    return true;
}


/**
 * mobe to the next token
 */
void Parser::move()
{
    m_token = m_next;
    m_next = m_lexer->next();
}


//
///**
// * Declaration  = [ "[" Attributes "]" ]
// *              ( VariableDecl
// *              | FunctionDecl
// *              | FunctionImpl
// *              )
// */
//AstDeclaration * Parser::declaration()
//{
//    AstAttributeList * attribs = nullptr;
//    AstDeclaration * decl = nullptr;
//    
//    // [ ... ]
//    if (accept(TokenType::BracketOpen)) {
//        attribs = attributesList();
//        if (!expect(TokenType::BracketClose)) {
//            // do error recovery?
//            return nullptr;
//        }
//    }
//
//    switch (m_token->type()) {
//        case TokenType::Dim:
//        case TokenType::Var:
//            decl = variableDecl();
//            break;
//        case TokenType::Declare:
//            decl = functionDecl();
//            break;
//        case TokenType::Function:
//        case TokenType::Sub:
//            decl = functionStmt();
//            break;
//        default:
//            // raise error
//            return nullptr;
//    }
//    
//    // add attribs
//    if (decl && attribs) decl->attribs.reset(attribs);
//    
//    // done
//    return decl;
//}
//
//
///**
// * StatementList = { Statement }
// */
//AstStmtList * Parser::statementList()
//{
//    auto ast = new AstStmtList();
//    
//    // { Statement }
//    while(!match(TokenType::EndOfFile)) {
//        // end of compound statements
//        switch(m_token->type()) {
//            case TokenType::End:
//            case TokenType::Else:
//            case TokenType::Next:
//                return ast;
//            default: {
//                auto stmt = statement();
//                if (stmt) ast->stmts.push_back(std::unique_ptr<AstStatement>(stmt));
//                if (!expect(TokenType::EndOfLine)) {
//                    // error recovery. iterate until end of lien
//                    return nullptr;
//                }
//            }
//        }
//    }
//    
//    return ast;
//}
//
//
///**
// * Statement    = VariableDecl
// *              | AssignStmt
// *              | FuncCallExpr
// *              | ReturnStmt
// *              | IfStmt
// *              | ForStmt
// */
//AstStatement * Parser::statement()
//{
//    AstStatement * stmt = nullptr;
//    switch (m_token->type()) {
//        case TokenType::Dim:
//        case TokenType::Var:
//            stmt = variableDecl();
//            break;
//        case TokenType::Return:
//            stmt = returnStmt();
//            break;
//        case TokenType::Identifier:
//            if (m_next->type() == TokenType::Assign) {
//                stmt = assignStmt();
//            } else /* if (m_next->type() == TokenType::ParenOpen) */ {
//                stmt = callStmt();
//            }
//            break;
//        // NOTE probably very wrong. at the moment assume
//        // that line starting with * is assignment to dereferenced
//        // pointer
//        // *ip = 10
//        case TokenType::Dereference:
//            stmt = assignStmt();
//            break;
//        case TokenType::If:
//            stmt = ifStmt();
//            break;
//        case TokenType::For:
//            stmt = forStmt();
//            break;
//        default:
//            // raise error. Expected statement
//            return nullptr;
//    }
//    
//    return stmt;
//}
//
//
///**
// * AssignStmt = Expression "=" Expression
// */
//AstAssignStmt * Parser::assignStmt()
//{
//    // left
//    m_expectAssign = true;
//    auto left = expression();
//    m_expectAssign = false;
//    if (left == nullptr) return nullptr;
//    
//    // =
//    if (!expect(TokenType::Assign)) return nullptr;
//    
//    // right
//    auto right = expression();
//    if (!right) return nullptr;
//    
//    // done
//    return new AstAssignStmt(left, right);
//}
//
//
///**
// * callStmt = id ( "(" FuncArgList ")" | FuncArgList ) 
// *
// * This doesn't conform to ebnf. Basically since parens are optional
// * for callstmt then need to distinguish between the following:
// * foo
// * foo(1, 2)
// * foo 1, 2
// * foo (1), 2
// * foo 1, (2)
// * foo (1), (2)
// */
//AstCallStmt * Parser::callStmt()
//{
//    // id
//    auto id = identifier();
//    if (!id) return nullptr;
//    
//    // "("
//    bool paren = accept(TokenType::ParenOpen);
//    
//    // ")"
//    if ((paren && accept(TokenType::ParenClose)) || (!paren && match(TokenType::EndOfLine))) {
//        return new AstCallStmt(new AstCallExpr(id, nullptr));
//    }
//    
//    auto args = new AstFuncArgList();
//    if (!args) return nullptr;
//    
//    // get 1st argument
//    auto expr = expression();
//    if (!expr) return nullptr;
//    args->args.push_back(std::unique_ptr<AstExpression>(expr));
//    
//    // is it arg list enclosed in ( and )
//    // or are the braces part of the expression?
//    if (paren && accept(TokenType::ParenClose)) {
//        paren = false;
//    }
//    
//    // rest of the arguments
//    while (accept(TokenType::Comma)) {
//        expr = expression();
//        if (!expr) return nullptr;
//        args->args.push_back(std::unique_ptr<AstExpression>(expr));
//    }
//    
//    // expect a closing ) ?
//    if (paren) {
//        if (!expect(TokenType::ParenClose)) {
//            // error recovery ?
//            return nullptr;
//        }
//    }
//    
//    // done
//    return new AstCallStmt(new AstCallExpr(id, args));
//}
//
//
///**
// * ifStmt   = "IF" Expression "THEN"
// *            StmtList
// *            ( "ELSE" ( ifStmt | StmtList "END" "IF" )
// *            | "END" "IF"
// *            )
// */
//AstIfStmt * Parser::ifStmt()
//{
//    // if
//    if (!expect(TokenType::If)) return nullptr;
//    
//    // expression
//    auto expr = expression();
//    if (!expr) return nullptr;
//    
//    // then
//    if (!expect(TokenType::Then)) return nullptr;
//    
//    // End Of Line
//    if (!expect(TokenType::EndOfLine)) return nullptr;
//    
//    // stmtlist if true
//    auto trueBlock = statementList();
//    if (!trueBlock) return nullptr;
//    AstStatement * falseBlock = nullptr;
//    
//    // else ?
//    bool expectEndIf = true;
//    if (accept(TokenType::Else)) {
//        // else if ?
//        if (match(TokenType::If)) {
//            falseBlock = ifStmt();
//            expectEndIf = false;
//        } else {
//            if (!expect(TokenType::EndOfLine)) return nullptr;
//            falseBlock = statementList();
//        }
//    }
//    
//    // if was Else IF
//    // then recursive ifStmt will parse the End If
//    if (expectEndIf) {
//        if (!expect(TokenType::End)) return nullptr;
//        if (!expect(TokenType::If)) return nullptr;
//    }
//    
//    // done
//    return new AstIfStmt(expr, trueBlock, falseBlock);
//}
//
//
///**
// * ForStmt  = "FOR" ( id [ "AS" TypeExpr ]
// *                  | "Var" id
// *                  | Expression
// *                  ) "=" Expression "TO" Expression [ "STEP" Expression ] \n
// *                StmtList
// *            "NEXT"
// */
//AstForStmt * Parser::forStmt()
//{
//    // "FOR"
//    if (!expect(TokenType::For)) return nullptr;
//    
//    // ( VariableDecl | AssignStmt )
//    AstStatement * stmt = nullptr;
//    if (match(TokenType::Var) || (match(TokenType::Identifier) && m_next->type() == TokenType::As)) {
//        auto dim = variableDecl(m_next->type() == TokenType::As);
//        if (!dim->expr) {
//            // raise error. Expected expression
//            return nullptr;
//        }
//        stmt = dim;
//    } else {
//        stmt = assignStmt();
//        if (!stmt) return nullptr;
//    }
//    
//    // "TO" expression
//    if (!expect(TokenType::To)) return nullptr;
//    auto end = expression();
//    
//    // ["STEP" expression]
//    AstExpression * step = nullptr;
//    if (accept(TokenType::Step)) {
//        step = expression();
//        if (!step) return nullptr;
//    }
//    
//    // end of line
//    if (!expect(TokenType::EndOfLine)) return nullptr;
//    
//    // StmtList
//    auto block = statementList();
//    if (!block) return nullptr;
//    
//    // "NEXT"
//    if (!expect(TokenType::Next)) return nullptr;
//    
//    // done
//    return new AstForStmt(stmt, end, step, block);
//}
//
//
///**
// * ReturnStmt = "RETURN" Expression
// */
//AstReturnStmt * Parser::returnStmt()
//{
//    if (!expect(TokenType::Return)) return nullptr;
//    
//    AstExpression * expr = nullptr;
//    if (!match(TokenType::EndOfLine)) {
//        expr = expression();
//        if (!expr) return nullptr;
//    }
//
//    return new AstReturnStmt(expr);
//}
//
//
///**
// * Expression = IntegerLiteral
// *            | FloatingPointLiteral
// *            | StringLiteral
// *            | CallExpr
// *            | id
// *            | AddressOf
// *            | Dereference
// *            | True
// *            | False
// *            | Null
// *            | "(" Expression ")"
// *            | Comparison
// *            | Mod
// *
// * for the moment use hackish way to add
// * a binary expression.
// * later will use an expression parser
// * that is not easily expressed in ebnf anyways
// */
//AstExpression * Parser::expression()
//{
//    AstExpression * expr = nullptr;
//    
//    // create scope
//    {
//        SCOPED_GUARD(m_expectAssign);
//        switch (m_token->type()) {
//            // IntegerLiteral
//            // FloatingPointLiteral
//            // StringLiteral
//            case TokenType::IntegerLiteral:
//            case TokenType::StringLiteral:
//            case TokenType::FloatingPointLiteral:
//                expr = new AstLiteralExpr(m_token);
//                if (!expr) return nullptr;
//                move();
//                break;
//            // true, false
//            case TokenType::True:
//            case TokenType::False:
//                expr = new AstLiteralExpr(m_token);
//                if (!expr) return nullptr;
//                move();
//                break;
//            // null
//            case TokenType::Null:
//                expr = new AstLiteralExpr(m_token);
//                if (!expr) return nullptr;
//                move();
//                break;
//            // CallExpr
//            // id
//            case TokenType::Identifier:
//                if (m_next->type() == TokenType::ParenOpen) {
//                    m_expectAssign = false;
//                    expr = callExpr();
//                    if (!expr) return nullptr;
//                    break;
//                } else {
//                    expr = identifier();
//                    if (!expr) return nullptr;
//                    break;
//                }
//            // AddressOf
//            case TokenType::AddressOf:
//                move();
//                expr = new AstAddressOfExpr(identifier());
//                if (!expr) return nullptr;
//                break;
//            // Dereference
//            case TokenType::Dereference:
//                move();
//                expr = new AstDereferenceExpr(expression());
//                if (!expr) return nullptr;
//                break;
//            // ( Expression )
//            case TokenType::ParenOpen:
//                move();
//                expr = expression();
//                if (!expr) return nullptr;
//                if (!expect(TokenType::ParenClose)) return nullptr;
//                break;
//            default:
//                // TODO: raise error
//                return nullptr;
//        }
//    }
//    
//    // =
//    if (!m_expectAssign) {
//        switch (m_token->type()) {
//            case TokenType::Assign:
//                m_token->type(TokenType::Equal);
//                // fall through!
//            case TokenType::Modulus:
//            case TokenType::NotEqual:
//            case TokenType::GreaterThan:
//            case TokenType::GreaterThanEqual:
//            case TokenType::LessThan:
//            case TokenType::LessThanEqual:
//            {
//                auto op = m_token;
//                move();
//                auto rhs = expression();
//                if (!rhs) return nullptr;
//                expr = new AstBinaryExpr(op, expr, rhs);
//                break;
//            }
//            default:
//                break;
//        }
//    }
//    
//    return expr;
//}
//
//
///**
// * CallExpr = id "(" [ funcArgList ] ")"
// */
//AstCallExpr * Parser::callExpr()
//{
//    // id
//    auto id = identifier();
//    if (!id) return nullptr;
//    AstFuncArgList * args = nullptr;
//    
//    // "("
//    if (!expect(TokenType::ParenOpen)) return nullptr;
//    
//    // [ funcArgList ]
//    if (!accept(TokenType::ParenClose)) {
//        args = funcArgList();
//        if (!args) return nullptr;
//        // ")"
//        if (!expect(TokenType::ParenClose)) return nullptr;
//    }
//    
//    return new AstCallExpr(id, args);
//}
//
//
///**
// * funcArgList = Expression { "," Expression }
// */
//AstFuncArgList * Parser::funcArgList()
//{
//    auto ast = new AstFuncArgList();
//    
//    do {
//        auto arg = expression();
//        if (!arg) {
//            // TODO raise error: expected expression
//            return nullptr;
//        }
//        ast->args.push_back(std::unique_ptr<AstExpression>(arg));
//    } while (accept(TokenType::Comma));
//    
//    return ast;
//}
//
//
///**
// * FunctionImpl = FuncSignature
// *              StatementList
// *              "END" "FUNCTION"
// */
//AstFunctionStmt * Parser::functionStmt()
//{
//    // FuncSignature
//    auto sig = funcSignature();
//    if (!sig) return nullptr;
//    
//    // no vararg support within lbc for now
//    if (sig->vararg) {
//        // TODO raise error: vararg not supported. yest
//        return nullptr;
//    }
//    if (!expect(TokenType::EndOfLine)) return nullptr;
//    
//    // StatementList
//    auto stmts = statementList();
//    if (!stmts) return nullptr;
//    
//    // END FUNCTION or SUB
//    if (!expect(TokenType::End)) return nullptr;
//    
//    if (sig->typeExpr) {
//        if (!expect(TokenType::Function)) return nullptr;
//    } else {
//        if (!expect(TokenType::Sub)) return nullptr;
//    }
//    
//    // done
//    return new AstFunctionStmt(sig, stmts);
//}
//
//
///**
// * FunctionDecl = "DECLARE" FuncSignature
// */
//AstFunctionDecl * Parser::functionDecl()
//{
//    if (!expect(TokenType::Declare)) return nullptr;
//    return new AstFunctionDecl(funcSignature());
//}
//
//
///**
// * FuncSignature = "FUNCTION" id "(" [ FuncParamList ] ")" "AS" TypeExpr
// */
//AstFuncSignature * Parser::funcSignature()
//{
//    // sub or function
//    bool isSub = accept(TokenType::Sub);
//    if (!isSub) {
//        if (!expect(TokenType::Function)) return nullptr;
//    }
//    
//    // id
//    auto id = identifier();
//    if (!id) {
//        // TODO: expected identifier
//        return nullptr;
//    }
//    bool vararg = false;
//    
//    // args
//    AstFuncParamList * params = nullptr;
//    
//    // make ( and ) optional
//    if (accept(TokenType::ParenOpen)) {
//        // [ FuncArgumentList ]
//        if (!accept(TokenType::ParenClose)) {
//            
//            params = funcParamList();
//            if (!params) {
//                // TODO raise error: expected parameters
//                return nullptr;
//            }
//            // ellipsis
//            if (accept(TokenType::Ellipsis)) {
//                vararg = true;
//            }
//            
//            // ")"
//            if (!expect(TokenType::ParenClose)) return nullptr;
//        }
//    }
//    
//    // return type
//    AstTypeExpr * type = nullptr;
//    
//    // if is a function then expect a type declaration
//    if (!isSub) {
//        // AS
//        if (!expect(TokenType::As)) return nullptr;
//        
//        // type
//        type = typeExpr();
//        if (!type) {
//            // TODO raise error: expect type expression
//            return nullptr;
//        }
//    }
//    
//    // done
//    return new AstFuncSignature(id, params, type, vararg);
//}
//
//
///**
// * FuncParamList = FuncParam { "," FuncParam }
// */
//AstFuncParamList * Parser::funcParamList()
//{
//    auto ast = new AstFuncParamList();
//    do {
//        if (match(TokenType::Ellipsis)) break;
//        auto param = funcParam();
//        if (!param) {
//            // TODO raise error: expected parameter
//            return nullptr;
//        }
//        ast->params.push_back(std::unique_ptr<AstFuncParam>(param));
//    } while (accept(TokenType::Comma));
//    
//    return ast;
//}
//
//
///**
// *  FuncParam = id "AS" TypeExpr
// */
//AstFuncParam * Parser::funcParam()
//{
//    // id
//    auto id = identifier();
//    if (!id) return nullptr;
//    // AS
//    if (!expect(TokenType::As)) return nullptr;
//    // type
//    auto type = typeExpr();
//    if (!type) return nullptr;
//    
//    // done
//    return new AstFuncParam(id, type);
//}
//
//
///**
// * VariableDecl = "VAR" id "=" Expression
// *              | "DIM" id "AS" TypeExpr [ "=" Expression ]
// */
//AstVarDecl * Parser::variableDecl(bool skipKw)
//{
//    // VAR ?
//    if (accept(TokenType::Var)) {
//        // id
//        auto id = identifier();
//        if (!id) return nullptr;
//        // "="
//        if (!expect(TokenType::Assign)) return nullptr;
//        // Expression
//        auto expr = expression();
//        if (!expr) return nullptr;
//        // done
//        return new AstVarDecl(id, nullptr, expr);
//    }
//    
//    if (!skipKw) {
//        // DIM
//        if (!expect(TokenType::Dim)) return nullptr;
//    }
//    // id
//    auto id = identifier();
//    if (!id) return nullptr;
//    
//    // AS
//    if (!expect(TokenType::As)) return nullptr;
//    
//    // TypeExpr
//    auto t = typeExpr();
//    if (!t) return nullptr;
//    
//    // [ "=" Expression ]
//    if (accept(TokenType::Assign)) {
//        auto expr = expression();
//        if (!expr) return nullptr;
//        return new AstVarDecl(id, t, expr);
//    }
//
//    // done
//    return new AstVarDecl(id, t, nullptr);
//}

