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
    _( Root )            \
    _( Statement )

//
// statement nodes. these extend the AstStatement
#define AST_STMT_NODES(_)   \
    _( StmtList )        \
    _( AssignStmt )      \
    _( ReturnStmt )      \
    _( CallStmt )

//
// declarations. These nodes can have attributes
// extend statement
#define AST_DECL_NODES(_)   \
    _( Declaration )     \
    _( VarDecl )         \
    _( FunctionDecl )    \
    _( FuncSignature )   \
    _( FunctionStmt )    \


//
// expression statements. these extend the AstExpression
// AstExpression is a root node, but it has content. so place it here
#define AST_EXPR_NODES(_)   \
    _( Expression )      \
    _( IdentExpr )       \
    _( LiteralExpr )     \
    _( CallExpr )        \
    _( CastExpr )        \
    _( AddressOfExpr )   \
    _( DereferenceExpr ) \
    _( BinaryExpr )

//
// ast nodes for attributes
#define AST_ATTRIB_NODES(_) \
    _( AttributeList )   \
    _( Attribute )       \
    _( AttribParamList )

//
// type declaration nodes
#define AST_TYPE_NODES(_)   \
    _( TypeExpr )

//
// Function argument and params
#define AST_FUNC_ARGS(_)    \
    _( FuncParamList )   \
    _( FuncArgList )     \
    _( FuncParam )


//
// Program node
#define AST_PROGRAM_NODES(_)\
    _( Program )

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
#define __AST_DECLARE_CLASS(C, ...) class Ast##C;
#define AST_DECLARE_CLASSES() \
    AST_ALL_NODES(__AST_DECLARE_CLASS)
