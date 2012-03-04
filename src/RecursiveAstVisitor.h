//
//  RecursiveAstVisitor.h
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

#include "AstVisitor.h"

namespace lbc {
	
	/**
	 * recursievly visit nodes. Statements, declarations
	 * and program
	 */
	struct RecursiveAstVisitor : AstVisitor
	{
		// create
		RecursiveAstVisitor(bool debug = false) : AstVisitor(debug) {}
		
		// AstProgram
		virtual void visit(AstProgram * ast);
		// AstStmtList
		virtual void visit(AstStmtList * ast);
		// AstDeclList
		virtual void visit(AstDeclList * ast);
		// AstFuncParamList
		virtual void visit(AstFuncParamList * ast);
		// AstFuncArgList
		virtual void visit(AstFuncArgList * ast);
		// AstAttributeList
		virtual void visit(AstAttributeList * ast);
		// AstAttribParamList
		virtual void visit(AstAttribParamList * ast);
		
		private:
		
	};

}
