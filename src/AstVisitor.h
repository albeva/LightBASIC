//
//  AstVisitor.h
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

#include "Ast.def.h"

namespace lbc {
	
	// forward declare Ast classes
	AST_DECLARE_CLASSES();
	
	/**
	 * ast tree visitor base class
	 */
	struct AstVisitor
	{
		// create new visitor
		AstVisitor(bool debug = false) : m_debug(debug) {}
		
		#define DECL_AST(C, ...) virtual void visit(C * ast) { \
			if(m_debug) std::cout << "Calling undefined visitor method for class " << #C << std::endl; \
		};
		AST_CONTENT_NODES(DECL_AST)
		#undef DECL_AST
		
	private:
		bool m_debug;
	};
	
}

