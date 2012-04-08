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
// * virtual destructor so that std::unique_ptr can work
// * virtual visitor method
#define IMPL_AST(C) \
    static boost::pool<> _pool##C(sizeof(Ast##C)); \
    void * Ast##C::operator new(size_t) { return _pool##C.malloc(); } \
    void Ast##C::operator delete(void * addr) { _pool##C.free(addr); } \
    void Ast##C::accept(AstVisitor * visitor) { visitor->visit(this); } \
    Ast##C::~Ast##C() {}
AST_CONTENT_NODES(IMPL_AST)
#undef IMPL_AST


/**
 * destroy root node
 */
AstRoot::~AstRoot() = default;


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
AstDeclaration::AstDeclaration(AstAttributeList * attrs) : attribs(attrs), symbol(nullptr) {}


// AstAttributeList
AstAttributeList::AstAttributeList() {}


// AstAttribute
AstAttribute::AstAttribute(AstIdentExpr * ID, AstAttribParamList * p) : id(ID), params(p) {}


// AstAttribParamList
AstAttribParamList::AstAttribParamList() {}


// AstVarDecl
AstVarDecl::AstVarDecl(AstIdentExpr * ID, AstTypeExpr * typ, AstExpression * e)
: id(ID), typeExpr(typ), expr(e) {}


// AstFunctionDecl
AstFunctionDecl::AstFunctionDecl(AstFuncSignature * sig) : signature(sig) {}


// AstFuncSignature
AstFuncSignature::AstFuncSignature(AstIdentExpr * ID, AstFuncParamList * p, AstTypeExpr * typ, bool va)
: id(ID), params(p), typeExpr(typ), vararg(va) {}


// AstFuncParamList
AstFuncParamList::AstFuncParamList() {}


// AstFuncParam
AstFuncParam::AstFuncParam(AstIdentExpr * ID, AstTypeExpr * typ)
: id(ID), typeExpr(typ) {}


// AstFunctionStmt
AstFunctionStmt::AstFunctionStmt(AstFuncSignature * sig, AstStmtList * s)
: signature(sig), stmts(s) {}


// AstStmtList
AstStmtList::AstStmtList() : symbolTable(nullptr) {}


// AstAssignStmt
AstAssignStmt::AstAssignStmt(AstExpression * l, AstExpression * r) : left(l), right(r) {}


// AstReturnStmt
AstReturnStmt::AstReturnStmt(AstExpression * e) : expr(e) {}


// AstCallStmt
AstCallStmt::AstCallStmt(AstCallExpr * e) : expr(e) {}


// AstIfStmt
AstIfStmt::AstIfStmt(AstExpression * e, AstStatement * t, AstStatement * f)
: expr(e), trueBlock(t), falseBlock(f) {}


// AstForStmt
AstForStmt::AstForStmt(AstStatement * s, AstExpression * e, AstExpression * st, AstStmtList * b)
: stmt(s), end(e), step(st), block(b) {}

//----------------------------------------------------------------------------------------------------------------------
// Expressions
//----------------------------------------------------------------------------------------------------------------------


// AstExpression
AstExpression::AstExpression() : type(nullptr) {}


// AstCastExpr
AstCastExpr::AstCastExpr(AstExpression * e, AstTypeExpr * t) : expr(e), typeExpr(t) {}


// AstAddressOfExpr
AstAddressOfExpr::AstAddressOfExpr(AstIdentExpr * ID) : id(ID) {}


// AstDereferenceExpr
AstDereferenceExpr::AstDereferenceExpr(AstExpression * e) : expr(e) {}


// AstIdentExpr
AstIdentExpr::AstIdentExpr(Token * t) : token(t) {}


// AstLiteralExpr
AstLiteralExpr::AstLiteralExpr(Token * t) : token(t) {}


// BinaryExpr
AstBinaryExpr::AstBinaryExpr(Token * op, AstExpression * l, AstExpression * r)
: token(op), lhs(l), rhs(r) {}


// AstCallExpr
AstCallExpr::AstCallExpr(AstIdentExpr * ID, AstFuncArgList * a) : id(ID), args(a) {}


// AstFuncArgList
AstFuncArgList::AstFuncArgList() {}


// AstTypeExpr
AstTypeExpr::AstTypeExpr(Token * t, int l)
: token(t), level(l) {}
