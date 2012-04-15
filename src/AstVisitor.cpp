//
//  AstVisitor.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 04/03/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "AstVisitor.h"
#include <iostream>
using namespace lbc;

// destructor
AstVisitor::~AstVisitor() = default;

// implement the methods
#define DECL_AST(C) \
    void AstVisitor::visit(Ast##C *) { \
        if(m_debug) std::cout << "Calling undefined visitor method for class Ast" << #C << std::endl; \
    }
AST_CONTENT_NODES(DECL_AST)

