//
//  Parser.h
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

#include "Ast.def.h"

namespace lbc {
    
    // forward decl
    class Context;
    class Lexer;
    class Token;
    class Source;
    
    // TokenType enum
    enum class TokenType : int;

    // forward declare Ast nodes
    AST_DECLARE_CLASSES()
    
    /**
     * This class parses the tokens into AST
     */
    class Parser : NonCopyable
    {
    public:
        
        // create new parser object
        Parser(Context & ctx);
        
        // parse and return AstProgram
        AstProgram * parse(const std::string & file);
        
    private:
        
        // top level declarations
        std::unique_ptr<AstDeclaration> declaration();
        std::unique_ptr<AstVarDecl> kwDim();
        std::unique_ptr<AstVarDecl> kwVar();
        
        std::unique_ptr<AstFunctionDecl> kwDeclare();
        std::unique_ptr<AstFuncSignature> funcSignature();
        std::unique_ptr<AstFuncParamList> funcParamList();
        std::unique_ptr<AstFuncParam> funcParam();
        
        // attributes
        std::unique_ptr<AstAttributeList>   attributes();
        std::unique_ptr<AstAttribute>       attribute();
        std::unique_ptr<AstAttribParamList> attribParamList();
        std::unique_ptr<AstLiteralExpr>     attribParam();
        
        // identifier
        std::unique_ptr<AstIdentExpr> identifier();
        
        // Type expression
        std::unique_ptr<AstTypeExpr> typeExpr();
        
        // Expression
        std::unique_ptr<AstExpression> expression();
        
//        // program
//        AstDeclaration * declaration();
//        AstAttributeList * attributesList();
//        AstAttribParamList * attribParamList();
//        AstAttribute * attribute();
//        AstIdentExpr * identifier();
//        AstLiteralExpr * attribParam();
//        AstVarDecl * variableDecl(bool skipKw = false);
//        AstTypeExpr * typeExpr();
//        AstFunctionDecl * functionDecl();
//        AstFuncSignature * funcSignature();
//        AstFuncParamList * funcParamList();
//        AstFuncParam * funcParam();
//        AstFunctionStmt * functionStmt();
//        AstStmtList * statementList();
//        AstStatement * statement();
//        AstAssignStmt * assignStmt();
//        AstCallStmt * callStmt();
//        AstIfStmt * ifStmt();
//        AstForStmt * forStmt();
//        AstCallExpr * callExpr();
//        AstExpression * expression();
//        AstReturnStmt * returnStmt();
//        AstFuncArgList * funcArgList();
        
        // match current token
        bool match(TokenType type) const;
        
        // Accept token and move to the next
        bool accept(TokenType type);
        
        // expect a token. Throw an error if doesn't match
        bool expect(TokenType type);
        
        // Move to the next one
        void move();
        
        // indicate if the is an error
        bool hasError() const { return m_hasError; }
        
        // tokens
        Context     & m_ctx;
        Token       * m_token;
        Token       * m_next;
        Lexer       * m_lexer;
        bool        m_expectAssign;
        bool        m_hasError;
    };
    
}
