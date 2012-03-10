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
    void C::accept(AstVisitor * visitor) { visitor->visit(this); }
AST_CONTENT_NODES(IMPL_AST)
#undef IMPL_AST


// AstProgram
AstProgram::AstProgram(const string & name) : name(name) {}
AstProgram::~AstProgram() {}


// AstDeclaration
AstDeclaration::AstDeclaration(AstAttributeList * attribs) : attribs(attribs), symbol(nullptr) {}
AstDeclaration::~AstDeclaration()
{
    if (attribs) delete attribs;
}


// AstAttributeList
AstAttributeList::AstAttributeList() {}
AstAttributeList::~AstAttributeList()
{
    for (auto t : attribs) delete t;
}


// AstAttribute
AstAttribute::AstAttribute(AstIdentExpr * id, AstAttribParamList * params) : id(id), params(params) {}
AstAttribute::~AstAttribute()
{
    if (id) delete id;
    if (params) delete params;
}


// AstAttribParamList
AstAttribParamList::AstAttribParamList() {}
AstAttribParamList::~AstAttribParamList()
{
    for (auto p : params) delete p;
}


// AstVarDecl
AstVarDecl::AstVarDecl(AstIdentExpr * id, AstTypeExpr * typeExpr) : id(id), typeExpr(typeExpr) {}
AstVarDecl::~AstVarDecl()
{
    if (id) delete id;
    if (typeExpr) delete typeExpr;
}


// AstFunctionDecl
AstFunctionDecl::AstFunctionDecl(AstFuncSignature * signature) : signature(signature) {}
AstFunctionDecl::~AstFunctionDecl()
{
    if (signature) delete signature;
}


// AstFuncSignature
AstFuncSignature::AstFuncSignature(AstIdentExpr * id, AstFuncParamList * params, AstTypeExpr * typeExpr, bool vararg)
: id(id), params(params), typeExpr(typeExpr), vararg(vararg) {}
AstFuncSignature::~AstFuncSignature()
{
    if (id) delete id;
    if (params) delete params;
    if (typeExpr) delete typeExpr;
}


// AstFuncParamList
AstFuncParamList::AstFuncParamList() {}
AstFuncParamList::~AstFuncParamList()
{
    for (auto p : params) delete p;
}


// AstFuncParam
AstFuncParam::AstFuncParam(AstIdentExpr * id, AstTypeExpr * typeExpr) : id(id), typeExpr(typeExpr) {}
AstFuncParam::~AstFuncParam()
{
    if (id) delete id;
    if (typeExpr) delete typeExpr;
}


// AstFunctionStmt
AstFunctionStmt::AstFunctionStmt(AstFuncSignature * signature, AstStmtList * stmts) : signature(signature), stmts(stmts) {}
AstFunctionStmt::~AstFunctionStmt()
{
    if (signature) delete signature;
    if (stmts) delete stmts;
}



// AstStmtList
AstStmtList::AstStmtList() : symbolTable(nullptr) {}
AstStmtList::~AstStmtList()
{
    for(auto s : stmts) delete s;
}


// AstAssignStmt
AstAssignStmt::AstAssignStmt(AstIdentExpr * id, AstExpression * expr) : id(id), expr(expr) {}
AstAssignStmt::~AstAssignStmt()
{
    if (id) delete id;
    if (expr) delete expr;
}


// AstReturnStmt
AstReturnStmt::AstReturnStmt(AstExpression * expr) : expr(expr) {}
AstReturnStmt::~AstReturnStmt()
{
    if (expr) delete expr;
}


// AstCallStmt
AstCallStmt::AstCallStmt(AstCallExpr * expr) : expr(expr) {}
AstCallStmt::~AstCallStmt()
{
    if (expr) delete expr;
}


// AstExpression
AstExpression::AstExpression() : type(nullptr) {}
AstExpression::~AstExpression() {}


// AstCastExpr
AstCastExpr::AstCastExpr(AstExpression * expr, AstTypeExpr * typeExpr) : expr(expr), typeExpr(typeExpr) {}
AstCastExpr::~AstCastExpr()
{
    if (expr) delete expr;
    if (typeExpr) delete typeExpr;
}


// AstIdentExpr
AstIdentExpr::AstIdentExpr(Token * token) : token(token) {}
AstIdentExpr::~AstIdentExpr()
{
    if (token) delete token;
}


// AstLiteralExpr
AstLiteralExpr::AstLiteralExpr(Token * token) : token(token) {}
AstLiteralExpr::~AstLiteralExpr()
{
    if (token) delete token;
}



// AstCallExpr
AstCallExpr::AstCallExpr(AstIdentExpr * id, AstFuncArgList * args) : id(id), args(args) {}
AstCallExpr::~AstCallExpr()
{
    if (id) delete id;
    if (args) delete args;
}


// AstFuncArgList
AstFuncArgList::AstFuncArgList() {}
AstFuncArgList::~AstFuncArgList()
{
    for (auto a : args) delete a;
}


// AstTypeExpr
AstTypeExpr::AstTypeExpr(Token * token, int level) : token(token), level(level) {}
AstTypeExpr::~AstTypeExpr()
{
    if (token) delete token;
}


