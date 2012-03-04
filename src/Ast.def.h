//
//  Ast.def.h
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

//
// base nodes that do not have visitors. all other
// ast nodes extend one of these
#define AST_ROOT_NODES(_)   \
    _( Ast )                \
    _( AstStatement )

//
// statement nodes. these extend the AstStatement
#define AST_STMT_NODES(_)   \
    _( AstStmtList )        \
    _( AstAssignStmt )      \
    _( AstReturnStmt )      \
    _( AstCallStmt )

//
// declarations. These nodes can have attributes
// extend statement
#define AST_DECL_NODES(_)   \
    _( AstDeclaration )     \
    _( AstVarDecl )         \
    _( AstFunctionDecl )    \
    _( AstFuncSignature )   \
    _( AstFunctionStmt )    \


//
// expression statements. these extend the AstExpression
// AstExpression is a root node, but it has content. so place it here
#define AST_EXPR_NODES(_)   \
    _( AstExpression )      \
    _( AstIdentExpr )       \
    _( AstLiteralExpr )     \
    _( AstCallExpr )

//
// ast nodes for attributes
#define AST_ATTRIB_NODES(_) \
    _( AstAttributeList )   \
    _( AstAttribute )       \
    _( AstAttribParamList )

//
// type declaration nodes
#define AST_TYPE_NODES(_)   \
    _( AstTypeExpr )

//
// Function argument and params
#define AST_FUNC_ARGS(_)    \
    _( AstFuncParamList )   \
    _( AstFuncArgList )     \
    _( AstFuncParam )


//
// Program node
#define AST_PROGRAM_NODES(_)\
    _( AstProgram )

//
// all content nodes (traversible by visitors)
// content nodes have a memory pool
#define AST_CONTENT_NODES(_)\
    AST_STMT_NODES(_)       \
    AST_DECL_NODES(_)       \
    AST_EXPR_NODES(_)       \
    AST_ATTRIB_NODES(_)     \
    AST_TYPE_NODES(_)       \
    AST_FUNC_ARGS(_)        \
    AST_PROGRAM_NODES(_)

//
// all nodes
#define AST_ALL_NODES(_)    \
    AST_ROOT_NODES(_)       \
    AST_CONTENT_NODES(_)

//
// forward declare all AST nodes
#define __AST_DECLARE_CLASS(C, ...) class C;
#define AST_DECLARE_CLASSES() \
    AST_ALL_NODES(__AST_DECLARE_CLASS)
    





