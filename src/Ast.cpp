//
//  Ast.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "Ast.h"
#include "Token.h"
#include "AstVisitor.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "Type.h"
using namespace lbc;

//
// implement the Ast nodes
// * pool allocation
// * virtual destructor so that unique_ptr can work
// * virtual visitor method
#define IMPL_AST(C, ...) \
	static boost::pool<> _pool##C(sizeof(C)); \
	void * C::operator new(size_t) { return _pool##C.malloc(); } \
	void C::operator delete(void * addr) { _pool##C.free(addr); } \
	C::~C() {} \
	void C::accept(AstVisitor * visitor) { visitor->visit(this); }
AST_CONTENT_NODES(IMPL_AST)
#undef IMPL_AST

// AstProgram
AstProgram::AstProgram()
{}


// AstDeclaration
AstDeclaration::AstDeclaration(AstAttributeList * attribs) : attribs(attribs)
{}


// AstDeclList
AstDeclList::AstDeclList(SymbolTable * table) : symbolTable(table)
{}


// AstAttributeList
AstAttributeList::AstAttributeList()
{}


// AstAttribute
AstAttribute::AstAttribute(AstIdentExpr * id, AstAttribParamList * params) : id(id), params(params)
{}


// AstAttribParamList
AstAttribParamList::AstAttribParamList()
{}


// AstVarDecl
AstVarDecl::AstVarDecl(AstIdentExpr * id, AstTypeExpr * type) : id(id), type(type)
{}


// AstFunctionDecl
AstFunctionDecl::AstFunctionDecl(AstFuncSignature * signature) : signature(signature)
{}


// AstFuncSignature
AstFuncSignature::AstFuncSignature(AstIdentExpr * id, AstFuncParamList * params, AstTypeExpr * type) : id(id), params(params), type(type)
{}


// AstFuncParamList
AstFuncParamList::AstFuncParamList()
{}

// AstFuncParam
AstFuncParam::AstFuncParam(AstIdentExpr * id, AstTypeExpr * type) : id(id), type(type) {}

// AstFunctionStmt
AstFunctionStmt::AstFunctionStmt(AstFuncSignature * signature, AstStmtList * stmts) : signature(signature), stmts(stmts)
{}


// AstStmtList
AstStmtList::AstStmtList(SymbolTable * table) : symbolTable(table)
{}


// AstAssignStmt
AstAssignStmt::AstAssignStmt(AstIdentExpr * id, AstExpression * expr) : id(id), expr(expr)
{}


// AstReturnStmt
AstReturnStmt::AstReturnStmt(AstExpression * expr) : expr(expr)
{}


// AstCallStmt
AstCallStmt::AstCallStmt(AstCallExpr * expr) : expr(expr)
{}


// AstExpression
AstExpression::AstExpression() {}

// AstIdentExpr
AstIdentExpr::AstIdentExpr(Token * token) : token(token)
{}


// AstLiteralExpr
AstLiteralExpr::AstLiteralExpr(Token * token) : token(token)
{}


// AstCallExpr
AstCallExpr::AstCallExpr(AstIdentExpr * id, AstFuncArgList * args) : id(id), args(args)
{}


// AstFuncArgList
AstFuncArgList::AstFuncArgList()
{}


// AstTypeExpr
AstTypeExpr::AstTypeExpr(Token * token, int level) : token(token), level(level)
{}
