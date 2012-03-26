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
#include "PrinterVisitor.h"
using namespace lbc;

//
// implement the Ast nodes
// * pool allocation
// * virtual destructor so that unique_ptr can work
// * virtual visitor method
#define IMPL_AST(C, ...) \
    static boost::pool<> _pool##C(sizeof(Ast##C)); \
    void * Ast##C::operator new(size_t) { return _pool##C.malloc(); } \
    void Ast##C::operator delete(void * addr) { _pool##C.free(addr); } \
    void Ast##C::accept(AstVisitor * visitor) { visitor->visit(this); } \
    Ast##C::~Ast##C() {}
AST_CONTENT_NODES(IMPL_AST)
#undef IMPL_AST


/**
 * print out the ast node source using PrinterVisitor
 */
void AstRoot::dump()
{
    PrinterVisitor p;
    this->accept(&p);
    std::cout << '\n';
}


// AstProgram
AstProgram::AstProgram(const string & name) : name(name) {}


// AstDeclaration
AstDeclaration::AstDeclaration(AstAttributeList * attribs) : attribs(attribs), symbol(nullptr) {}


// AstAttributeList
AstAttributeList::AstAttributeList() {}


// AstAttribute
AstAttribute::AstAttribute(AstIdentExpr * id, AstAttribParamList * params) : id(id), params(params) {}


// AstAttribParamList
AstAttribParamList::AstAttribParamList() {}


// AstVarDecl
AstVarDecl::AstVarDecl(AstIdentExpr * id, AstTypeExpr * typeExpr, AstExpression * expr)
: id(id), typeExpr(typeExpr), expr(expr) {}


// AstFunctionDecl
AstFunctionDecl::AstFunctionDecl(AstFuncSignature * signature) : signature(signature) {}


// AstFuncSignature
AstFuncSignature::AstFuncSignature(AstIdentExpr * id, AstFuncParamList * params, AstTypeExpr * typeExpr, bool vararg)
: id(id), params(params), typeExpr(typeExpr), vararg(vararg) {}


// AstFuncParamList
AstFuncParamList::AstFuncParamList() {}


// AstFuncParam
AstFuncParam::AstFuncParam(AstIdentExpr * id, AstTypeExpr * typeExpr) : id(id), typeExpr(typeExpr) {}


// AstFunctionStmt
AstFunctionStmt::AstFunctionStmt(AstFuncSignature * signature, AstStmtList * stmts) : signature(signature), stmts(stmts) {}


// AstStmtList
AstStmtList::AstStmtList() : symbolTable(nullptr) {}


// AstAssignStmt
AstAssignStmt::AstAssignStmt(AstExpression * left, AstExpression * right) : left(left), right(right) {}


// AstReturnStmt
AstReturnStmt::AstReturnStmt(AstExpression * expr) : expr(expr) {}


// AstCallStmt
AstCallStmt::AstCallStmt(AstCallExpr * expr) : expr(expr) {}


// AstIfStmt
AstIfStmt::AstIfStmt(AstExpression * expr, AstStatement * trueBlock, AstStatement * falseBlock)
: expr(expr), trueBlock(trueBlock), falseBlock(falseBlock) {}


//----------------------------------------------------------------------------------------------------------------------
// Expressions
//----------------------------------------------------------------------------------------------------------------------


// AstExpression
AstExpression::AstExpression() : type(nullptr) {}


// AstCastExpr
AstCastExpr::AstCastExpr(AstExpression * expr, AstTypeExpr * typeExpr) : expr(expr), typeExpr(typeExpr) {}


// AstAddressOfExpr
AstAddressOfExpr::AstAddressOfExpr(AstIdentExpr * id) : id(id) {}


// AstDereferenceExpr
AstDereferenceExpr::AstDereferenceExpr(AstExpression * expr) : expr(expr) {}


// AstIdentExpr
AstIdentExpr::AstIdentExpr(Token * token) : token(token) {}


// AstLiteralExpr
AstLiteralExpr::AstLiteralExpr(Token * token) : token(token) {}


// BinaryExpr
AstBinaryExpr::AstBinaryExpr(Token * op, AstExpression * lhs, AstExpression * rhs)
: token(op), lhs(lhs), rhs(rhs) {}


// AstCallExpr
AstCallExpr::AstCallExpr(AstIdentExpr * id, AstFuncArgList * args) : id(id), args(args) {}


// AstFuncArgList
AstFuncArgList::AstFuncArgList() {}


// AstTypeExpr
AstTypeExpr::AstTypeExpr(Token * token, int level) : token(token), level(level) {}
