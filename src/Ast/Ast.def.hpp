//
// Created by Albert Varaksin on 05/07/2020.
//
#pragma once

// clang-format off

//----------------------------------------
// Root nodes serving as parents for others
//----------------------------------------
#define AST_ROOT_NODES(_) \
    _( Root ) \
    _( Stmt ) \
    _( Expr ) \
    _( Decl )

//----------------------------------------
// Misc. nodes extending AstRoot
//----------------------------------------
#define AST_BASIC_NODES(_) \
    _( Module        ) \
    _( ExprList      ) \
    _( Attribute     ) \
    _( AttributeList ) \
    _( FuncParamList )

//----------------------------------------
// Statements nodes extending AstStmt
//----------------------------------------
#define AST_STMT_NODES(_) \
    _( Import           ) \
    _( StmtList         ) \
    _( ExprStmt         ) \
    _( FuncStmt         ) \
    _( ReturnStmt       ) \
    _( IfStmt           ) \
    _( ForStmt          ) \
    _( DoLoopStmt       ) \
    _( ContinuationStmt )

#define AST_STMT_RANGE(_) _(Import, ContinuationStmt)

//----------------------------------------
// Declaration nodes extending AstDecl
//----------------------------------------
#define AST_DECL_NODES(_) \
    _( VarDecl       ) \
    _( FuncDecl      ) \
    _( FuncParamDecl ) \
    _( TypeDecl      )

#define AST_DECL_RANGE(_) _(VarDecl, TypeDecl)

//----------------------------------------
// Type nodes extending AstRoot
//----------------------------------------
#define AST_TYPE_NODES(_) \
    _( TypeExpr )

#define AST_TYPE_RANGE(_) _(TypeExpr, TypeExpr)

//----------------------------------------
// Expression nodes extending AstExpr
//----------------------------------------
#define AST_EXPR_NODES(_) \
    _( AssignExpr   ) \
    _( IdentExpr    ) \
    _( CallExpr     ) \
    _( LiteralExpr  ) \
    _( UnaryExpr    ) \
    _( BinaryExpr   ) \
    _( CastExpr     ) \
    _( IfExpr       ) \
    _( Dereference  ) \
    _( AddressOf    ) \
    _( MemberAccess )

#define AST_EXPR_RANGE(_) _(AssignExpr, MemberAccess)

//----------------------------------------
// All content nodes
//----------------------------------------
#define AST_CONTENT_NODES(_) \
    AST_BASIC_NODES(_)  \
    AST_STMT_NODES(_)   \
    AST_DECL_NODES(_)   \
    AST_TYPE_NODES(_)   \
    AST_EXPR_NODES(_)

//----------------------------------------
// Forward declare nodes
//----------------------------------------
#define AST_FORWARD_DECLARE_IMPL(C) struct Ast##C;
#define AST_FORWARD_DECLARE()                \
    AST_ROOT_NODES(AST_FORWARD_DECLARE_IMPL) \
    AST_CONTENT_NODES(AST_FORWARD_DECLARE_IMPL)
