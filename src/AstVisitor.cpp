//
//  AstVisitor.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 04/03/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "AstVisitor.h"
using namespace lbc;

// implement the methods
#define DECL_AST(C, ...) \
    void AstVisitor::visit(C * ast) { \
        if(m_debug) std::cout << "Calling undefined visitor method for class " << #C << std::endl; \
    };
AST_CONTENT_NODES(DECL_AST)
