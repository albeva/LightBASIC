//
//  RecursiveAstVisitor.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "RecursiveAstVisitor.h"
#include "Ast.h"
using namespace lbc;


//
// AstProgram
void RecursiveAstVisitor::visit(AstProgram * ast)
{
    for (auto decl : ast->decls) decl->accept(this);
}


//
// AstStmtList
void RecursiveAstVisitor::visit(AstStmtList * ast)
{
    for (auto stmt : ast->stmts) stmt->accept(this);
}


//
// AstFuncParamList
void RecursiveAstVisitor::visit(AstFuncParamList * ast)
{
    for (auto stmt : ast->params) stmt->accept(this);
}

//
// AstFuncArgList
void RecursiveAstVisitor::visit(AstFuncArgList * ast)
{
    for (auto arg : ast->args) arg->accept(this);
}


//
// AstAttributeList
void RecursiveAstVisitor::visit(AstAttributeList * ast)
{
    for (auto attr : ast->attribs) attr->accept(this);
}


//
// AstAttribParamList
void RecursiveAstVisitor::visit(AstAttribParamList * ast)
{
    for (auto param : ast->params) param->accept(this);
}
