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
	
	// TokenType enum
	enum class TokenType : int;

	// forward declare Ast nodes
	AST_DECLARE_CLASSES();
	
	/**
	 * This class parses the tokens into AST
	 */
	struct Parser : NonCopyable
	{
		// create new parser object
		Parser(const shared_ptr<Context> & ctx);
		
		// parse and return AstProgram
		AstProgram * parse();
		
	private:
		
		// program
		AstDeclList * declList();
		AstDeclaration * declaration();
		AstAttributeList * attributesList();
		AstAttribParamList * attribParamList();
		AstAttribute * attribute();
		AstIdentExpr * identifier();
		AstLiteralExpr * attribParam();
		AstVarDecl * variableDecl();
		AstTypeExpr * typeExpr();
		AstFunctionDecl * functionDecl();
		AstFuncSignature * funcSignature();
		AstFuncParamList * funcParamList();
		AstFuncParam * funcParam();
		AstFunctionStmt * functionStmt();
		AstStmtList * statementList();
		AstStatement * statement();
		AstAssignStmt * assignStmt();
		AstCallStmt * callStmt();
		AstCallExpr * callExpr();
		AstExpression * expression();
		AstReturnStmt * returnStmt();
		AstFuncArgList * funcArgList();
		
        // match current token
        bool match(TokenType type) const;
        
        // Accept token and move to the next
        bool accept(TokenType type);
        
        // expect a token. Throw an error if doesn't match
        void expect(TokenType type);
        
        // Move to the next one
        void move();
        
        // tokens
		shared_ptr<Context>	m_ctx;
        Token				* m_token;
        Token				* m_next;
		Lexer				* m_lexer;
	};
	
}
