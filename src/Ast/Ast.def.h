//
// Created by Albert on 05/07/2020.
//
#pragma once

#define AST_ROOT_NODES(_) \
    _( Root ) \
    _( Stmt ) \
    _( Expr ) \
    _( Decl )

// statements
#define AST_STMT_NODES(_) \
    _( Program    ) \
    _( StmtList   ) \
    _( AssignStmt ) \
    _( ExprStmt   )

#define AST_DECL_NODES(_) \
    _( VarDecl )

// all expressions
#define AST_EXPR_NODES(_) \
    _( IdentExpr   ) \
    _( CallExpr    ) \
    _( LiteralExpr )

// all nodes
#define AST_CONTENT_NODES(_) \
    AST_STMT_NODES(_) \
    AST_DECL_NODES(_) \
    AST_EXPR_NODES(_)

// forward declare all AST nodes
#define __AST_DECLARE_CLASS(C) class Ast##C;
#define AST_FORWARD_DECLARE() \
    AST_ROOT_NODES(__AST_DECLARE_CLASS) \
    AST_CONTENT_NODES(__AST_DECLARE_CLASS)
