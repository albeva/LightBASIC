//
//  SemanticAnalyser.h
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

#include "Ast.h"
#include "RecursiveAstVisitor.h"

namespace lbc {
	
	// forward reference
	class SymbolTable;
	class Symbol;
	class Type;

	/**
	 * Perform semantic analyses on the Ast tree
	 * and build symbol table along the way
	 */
	struct SemanticAnalyser : RecursiveAstVisitor
	{
		SemanticAnalyser();
		
		// AstDeclList
		virtual void visit(AstProgram * ast);
		
		// AstFunctionDecl
		virtual void visit(AstFunctionDecl * ast);
		
		// AstFuncSignature
		virtual void visit(AstFuncSignature * ast);
		
		// AstFuncParam
		virtual void visit(AstFuncParam * ast);
		
		// AstTypeExpr
		virtual void visit(AstTypeExpr * ast);
		
		// AstVarDecl
		virtual void visit(AstVarDecl * ast);
		
		// AstAssignStmt
		virtual void visit(AstAssignStmt * ast);
		
		// AstLiteralExpr
		virtual void visit(AstLiteralExpr * ast);
		
		// AstCallStmt
		virtual void visit(AstCallStmt * ast);
		
		// AstCallExpr
		virtual void visit(AstCallExpr * ast);
		
		// AstIdentExpr
		virtual void visit(AstIdentExpr * ast);
		
		// AstFunctionStmt
		virtual void visit(AstFunctionStmt * ast);
		
		// AstReturnStmt
		virtual void visit(AstReturnStmt * ast);
		
		// AstAttribute
		virtual void visit(AstAttribute * ast);
		
	private:
		SymbolTable * m_table;
		Symbol * m_symbol;
		shared_ptr<Type> m_type;
	};

}
