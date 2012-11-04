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
#include "MemoryPool.h"
#include <iostream>
using namespace lbc;

//
// implement the Ast nodes
// * pool allocation
// * virtual destructor so that std::unique_ptr can work
// * virtual visitor method
#define IMPL_AST(C) \
    static MemoryPool<Ast##C> _pool##C; \
    void * Ast##C::operator new(size_t) { return (void *)_pool##C.allocate(); } \
    void Ast##C::operator delete(void * addr) { _pool##C.deallocate(addr); } \
    void Ast##C::accept(AstVisitor * visitor) { visitor->visit(this); } \
    Ast##C::~Ast##C() {}
AST_CONTENT_NODES(IMPL_AST)
#undef IMPL_AST


/**
 * destroy root node
 */
AstRoot::~AstRoot() = default;

/**
 * AstStatement
 */
AstStatement::AstStatement() = default;
AstStatement::~AstStatement() = default;


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
AstProgram::AstProgram(const std::string & n) : name(n) {}


// AstDeclaration
AstDeclaration::AstDeclaration(std::unique_ptr<AstAttributeList> attrs) :
attribs(std::move(attrs)), symbol(nullptr) {}


// AstAttributeList
AstAttributeList::AstAttributeList() {}


// AstAttribute
AstAttribute::AstAttribute(std::unique_ptr<AstIdentExpr> _id,
                           std::unique_ptr<AstAttribParamList> _params)
: id(std::move(_id)), params(std::move(_params))
{
}


// AstAttribParamList
AstAttribParamList::AstAttribParamList() {}


// AstVarDecl
AstVarDecl::AstVarDecl(std::unique_ptr<AstIdentExpr> ID,
                       std::unique_ptr<AstTypeExpr> typ,
                       std::unique_ptr<AstExpression> e)
: id(std::move(ID)), typeExpr(std::move(typ)), expr(std::move(e)) {}


// AstFunctionDecl
AstFunctionDecl::AstFunctionDecl(std::unique_ptr<AstFuncSignature> sig)
: signature(std::move(sig)) {}


// AstFuncSignature
AstFuncSignature::AstFuncSignature(std::unique_ptr<AstIdentExpr> ID,
                                   std::unique_ptr<AstFuncParamList> p,
                                   std::unique_ptr<AstTypeExpr> typ,
                                   bool va)
: id(std::move(ID)), params(std::move(p)), typeExpr(std::move(typ)), vararg(va) {}


// AstFuncParamList
AstFuncParamList::AstFuncParamList() {}


// AstFuncParam
AstFuncParam::AstFuncParam(std::unique_ptr<AstIdentExpr> ID,
                           std::unique_ptr<AstTypeExpr> typ)
: id(std::move(ID)), typeExpr(std::move(typ)) {}


// AstFunctionStmt
AstFunctionStmt::AstFunctionStmt(std::unique_ptr<AstFuncSignature> sig,
                                 std::unique_ptr<AstStmtList> s)
: signature(std::move(sig)), stmts(std::move(s)) {}


// AstStmtList
AstStmtList::AstStmtList() : symbolTable(nullptr) {}


// AstAssignStmt
AstAssignStmt::AstAssignStmt(std::unique_ptr<AstExpression> l,
                             std::unique_ptr<AstExpression> r)
: left(std::move(l)), right(std::move(r)) {}


// AstReturnStmt
AstReturnStmt::AstReturnStmt(std::unique_ptr<AstExpression> e)
: expr(std::move(e)) {}


// AstCallStmt
AstCallStmt::AstCallStmt(std::unique_ptr<AstCallExpr> e)
: expr(std::move(e)) {}


// AstIfStmt
AstIfStmt::AstIfStmt(std::unique_ptr<AstExpression> e,
                     std::unique_ptr<AstStatement> t,
                     std::unique_ptr<AstStatement> f)
: expr(std::move(e)), trueBlock(std::move(t)), falseBlock(std::move(f)) {}


// AstForStmt
AstForStmt::AstForStmt(std::unique_ptr<AstStatement> s,
                       std::unique_ptr<AstExpression> e,
                       std::unique_ptr<AstExpression> st,
                       std::unique_ptr<AstStmtList> b)
: stmt(std::move(s)), end(std::move(e)), step(std::move(st)), block(std::move(b)) {}

//----------------------------------------------------------------------------------------------------------------------
// Expressions
//----------------------------------------------------------------------------------------------------------------------


// AstExpression
AstExpression::AstExpression() : type(nullptr) {}


// AstCastExpr
AstCastExpr::AstCastExpr(std::unique_ptr<AstExpression> e,
                         std::unique_ptr<AstTypeExpr> t)
: expr(std::move(e)), typeExpr(std::move(t)) {}


// AstAddressOfExpr
AstAddressOfExpr::AstAddressOfExpr(std::unique_ptr<AstIdentExpr> ID)
: id(std::move(ID)) {}


// AstDereferenceExpr
AstDereferenceExpr::AstDereferenceExpr(std::unique_ptr<AstExpression> e)
: expr(std::move(e)) {}


// AstIdentExpr
AstIdentExpr::AstIdentExpr(Token * t)
: token(t) {}


// AstLiteralExpr
AstLiteralExpr::AstLiteralExpr(Token * t)
: token(t) {}


// BinaryExpr
AstBinaryExpr::AstBinaryExpr(Token * op,
                             std::unique_ptr<AstExpression> l,
                             std::unique_ptr<AstExpression> r)
: token(op), lhs(std::move(l)), rhs(std::move(r)) {}


// AstCallExpr
AstCallExpr::AstCallExpr(std::unique_ptr<AstIdentExpr> ID,
                         std::unique_ptr<AstFuncArgList> a)
: id(std::move(ID)), args(std::move(a)) {}


// AstFuncArgList
AstFuncArgList::AstFuncArgList() {}


// AstTypeExpr
AstTypeExpr::AstTypeExpr(Token * t, int l)
: token(t), level(l) {}
