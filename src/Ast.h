//
//  Ast.h
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once
#include "Ast.def.h"

namespace lbc {
	
	// forward declare Ast classes
	AST_DECLARE_CLASSES();
	
	// forward declare the visitor
	class AstVisitor;
	class Token;
	class Symbol;
	class SymbolTable;
	class Type;
	
	//
	// declare Ast node. define visitor method, mempory pooling and virtual destructor
	#define DECLARE_AST(C) \
		virtual void accept(AstVisitor * visitor); \
		void * operator new(size_t); \
		void operator delete(void *); \
		virtual ~C();
	
	
	//--------------------------------------------------------------------------
	// Basic Nodes
	//--------------------------------------------------------------------------
	
	/**
	 * This is the base node for the AST tree
	 */
	struct Ast : NonCopyable
	{
		// visitor pattern
		virtual void accept(AstVisitor * visitor) = 0;
		
		// virtual destructor
		virtual ~Ast() = default;
	};
	
	
	/**
	 * base node for statements
	 */
	struct AstStatement : Ast {};
	
	
	/**
	 * program root
	 */
	struct AstProgram : Ast {
		// create
		AstProgram(SymbolTable * table = nullptr);
		// list of declaration lists
		boost::ptr_vector<AstDeclaration> decls;
		// symbol table
		shared_ptr<SymbolTable> symbolTable;
		// content node
		DECLARE_AST(AstProgram);
	};


	//--------------------------------------------------------------------------
	// Declarations
	//--------------------------------------------------------------------------
	
	
	/**
	 * Declaration
	 */
	struct AstDeclaration : AstStatement
	{
		// create
		AstDeclaration(AstAttributeList * attribs = nullptr);
		// attribs
		unique_ptr<AstAttributeList> attribs;
		// declaration symbol
		Symbol * symbol;
		// content node
		DECLARE_AST(AstDeclaration);
	};
	
	
	/**
	 * attribute list
	 */
	struct AstAttributeList : Ast
	{
		// create
		AstAttributeList();
		// hold list of attributes
		boost::ptr_vector<AstAttribute> attribs;
		// content node
		DECLARE_AST(AstAttributeList);
	};
	
	
	/**
	 * attribute node
	 */
	struct AstAttribute : Ast
	{
		// create
		AstAttribute(AstIdentExpr * id = nullptr, AstAttribParamList * params = nullptr);
		// attribute id
		unique_ptr<AstIdentExpr> id;
		// attribute params
		unique_ptr<AstAttribParamList> params;
		// content node
		DECLARE_AST(AstAttribute);
	};
	
	
	/**
	 * list of attribute parameters
	 */
	struct AstAttribParamList : Ast
	{
		// create
		AstAttribParamList();
		// attribute params
		boost::ptr_vector<AstLiteralExpr> params;
		// content node
		DECLARE_AST(AstAttribParamList);
	};
	
	
	/**
	 * variable declaration
	 */
	struct AstVarDecl : AstDeclaration
	{
		// create
		AstVarDecl(AstIdentExpr * id = nullptr, AstTypeExpr * typeExpr = nullptr);
		// variable id
		unique_ptr<AstIdentExpr> id;
		// variable type
		unique_ptr<AstTypeExpr> typeExpr;
		// content node
		DECLARE_AST(AstVarDecl);
	};
	
	
	/**
	 * function declaration
	 */
	struct AstFunctionDecl : AstDeclaration
	{
		// create
		AstFunctionDecl(AstFuncSignature * signature = nullptr);
		// function signature
		unique_ptr<AstFuncSignature> signature;
		// content node
		DECLARE_AST(AstFunctionDecl);
	};
	
	
	/**
	 * function signature part
	 */
	struct AstFuncSignature : Ast
	{
		// create
		AstFuncSignature(AstIdentExpr * id = nullptr, AstFuncParamList * args = nullptr, AstTypeExpr * typeExpr = nullptr);
		// function id
		unique_ptr<AstIdentExpr> id;
		// function parameters
		unique_ptr<AstFuncParamList> params;
		// function type
		unique_ptr<AstTypeExpr> typeExpr;
		// content node
		DECLARE_AST(AstFuncSignature);
	};
	
	
	/**
	 * function parameter list
	 */
	struct AstFuncParamList : Ast
	{
		// create
		AstFuncParamList();
		// list of function parameters
		boost::ptr_vector<AstFuncParam> params;
		// content node
		DECLARE_AST(AstFuncParamList);
	};
	
	
	/**
	 * Function parameter
	 */
	struct AstFuncParam : AstDeclaration
	{
		// create
		AstFuncParam(AstIdentExpr * id = nullptr, AstTypeExpr * typeExpr = nullptr);
		// variable id
		unique_ptr<AstIdentExpr> id;
		// variable type
		unique_ptr<AstTypeExpr> typeExpr;
		// content node
		DECLARE_AST(AstFuncParam);
	};
	
	
	/**
	 * function implementation
	 */
	struct AstFunctionStmt : AstDeclaration
	{
		// create
		AstFunctionStmt(AstFuncSignature * signature = nullptr, AstStmtList * stmts = nullptr);
		// function signature
		unique_ptr<AstFuncSignature> signature;
		// function body
		unique_ptr<AstStmtList> stmts;
		// content node
		DECLARE_AST(AstFunctionStmt);
	};
	
	
	//--------------------------------------------------------------------------
	// Statements
	//--------------------------------------------------------------------------
	
	
	/**
	 * Root node representing whole program
	 */
	struct AstStmtList : Ast
	{
		// create
		AstStmtList();
		// list of statements
		boost::ptr_vector<AstStatement> stmts;
		// associated symbol table.
		SymbolTable * symbolTable;
		// content node
		DECLARE_AST(AstStmtList);
	};
	
	
	/**
	 * assigment statement
	 */
	struct AstAssignStmt : AstStatement
	{
		// create 
		AstAssignStmt(AstIdentExpr * id = nullptr, AstExpression * expr = nullptr);
		// assignee id
		unique_ptr<AstIdentExpr> id;
		// the expression
		unique_ptr<AstExpression> expr;
		// content node
		DECLARE_AST(AstAssignStmt);
	};
	
	
	/**
	 * RETURN statement
	 */
	struct AstReturnStmt : AstStatement
	{
		// create
		AstReturnStmt(AstExpression * expr = nullptr);
		// the expression
		unique_ptr<AstExpression> expr;
		// content node
		DECLARE_AST(AstReturnStmt);
	};
	
	
	/**
	 * call statement. wrap the call expression
	 */
	struct AstCallStmt : AstStatement
	{
		// create
		AstCallStmt(AstCallExpr * expr = nullptr);
		// the call expression
		unique_ptr<AstCallExpr> expr;
		// content node
		DECLARE_AST(AstCallStmt);
	};
	
	//--------------------------------------------------------------------------
	// Expressions
	//--------------------------------------------------------------------------
	
	/**
	 * base node for expressions
	 */
	struct AstExpression : Ast
	{
		// create
		AstExpression();
		// expression type
		shared_ptr<Type> type;
		// content node
		DECLARE_AST(AstExpression);
	};
	
	
	/**
	 * identifier node
	 */
	struct AstIdentExpr : AstExpression
	{
		// create
		AstIdentExpr(Token * token = nullptr);
		// the id token
		unique_ptr<Token> token;
		// content node
		DECLARE_AST(AstIdentExpr);
	};
	
	
	/**
	 * base for literal expressions (string, number)
	 */
	struct AstLiteralExpr : AstExpression
	{
		// create
		AstLiteralExpr(Token * token = nullptr);
		// the value token
		unique_ptr<Token> token;
		// content node
		DECLARE_AST(AstLiteralExpr);
	};
	
	
	/**
	 * call expression
	 */
	struct AstCallExpr : AstExpression
	{
		// create
		AstCallExpr(AstIdentExpr * id = nullptr, AstFuncArgList * args = nullptr);
		// callee id
		unique_ptr<AstIdentExpr> id;
		// parameters
		unique_ptr<AstFuncArgList> args;
		// content node
		DECLARE_AST(AstCallExpr);
	};
	
	
	/**
	 * function arguments
	 */
	struct AstFuncArgList : Ast
	{
		// create
		AstFuncArgList();
		// list of arguments
		boost::ptr_vector<AstExpression> args;
		// content node
		DECLARE_AST(AstFuncArgList);
	};
	
	
	//--------------------------------------------------------------------------
	// Type
	//--------------------------------------------------------------------------
	
	/**
	 * type declarator
	 */
	struct AstTypeExpr : Ast
	{
		// create
		AstTypeExpr(Token * token = nullptr, int level = 0);
		// type id
		unique_ptr<Token> token;
		// dereference level
		int level;
		// content node
		DECLARE_AST(AstTypeExpr);
	};
	
}
