//
//  PrinterVisitor.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "PrinterVisitor.h"
#include "Token.h"
using namespace lbc;

//
// create the printer
PrinterVisitor::PrinterVisitor() : m_indent(0) {}


//
// indent by number of spaces
string PrinterVisitor::indent(int change)
{
    m_indent += change;
    return string(m_indent * 4, ' ');
}


//
// AstAssignStmt
void PrinterVisitor::visit(AstAssignStmt * ast)
{
    std::cout << indent();
    if (ast->left) ast->left->accept(this);
    std::cout << " = ";
    if (ast->right) ast->right->accept(this);
    std::cout << std::endl;
}


//
// AstReturnStmt
void PrinterVisitor::visit(AstReturnStmt * ast)
{
    std::cout << indent() << "RETURN ";
    if (ast->expr) ast->expr->accept(this);
    std::cout << std::endl;
}


//
// AstCallStmt
void PrinterVisitor::visit(AstCallStmt * ast)
{
    std::cout << indent();
    if (ast->expr) ast->expr->accept(this);
    std::cout << std::endl;
}


//
// AstDeclaration
void PrinterVisitor::visit(AstDeclaration * ast)
{
    std::cout << "SHOULD NEVER GET HERE!";
}


//
// AstVarDecl
void PrinterVisitor::visit(AstVarDecl * ast)
{
    std::cout << indent() << "DIM ";
    if (ast->id) ast->id->accept(this);
    std::cout << " AS ";
    if (ast->typeExpr) ast->typeExpr->accept(this);
    std::cout << std::endl;
}


//
// AstFunctionDecl
void PrinterVisitor::visit(AstFunctionDecl * ast)
{
    if (ast->attribs) ast->attribs->accept(this);
    
    std::cout << indent() << "DECLARE ";
    if (ast->signature) ast->signature->accept(this);
    std::cout << std::endl;
}


//
// AstFuncSignature
void PrinterVisitor::visit(AstFuncSignature * ast)
{
    std::cout << "FUNCTION ";
    if (ast->id) ast->id->accept(this);
    std::cout << "(";
    if (ast->params) ast->params->accept(this);
    if (ast->vararg) {
        if (ast->params && ast->params->params.size()) std::cout << ", ";
        std::cout << "...";
    }
    std::cout << ") AS ";
    if (ast->typeExpr) ast->typeExpr->accept(this);
}


//
// AstFunctionStmt
void PrinterVisitor::visit(AstFunctionStmt * ast)
{
    if (ast->attribs) ast->attribs->accept(this);
    
    std::cout << indent();
    if (ast->signature) ast->signature->accept(this);
    std::cout << std::endl;
    m_indent++;
    if (ast->stmts) ast->stmts->accept(this);
    m_indent--;
    std::cout << indent() << "END FUNCTION" << std::endl;
}


//
// AstCastExpr
void PrinterVisitor::visit(AstCastExpr * ast)
{
    // if no typeExpr then this is an implicit cast
    if (!ast->typeExpr) {
        if (ast->expr) ast->expr->accept(this);
    } else {
        std::cout << "CAST(";
        if (ast->typeExpr) ast->typeExpr->accept(this);
        std::cout << ", ";
        if (ast->expr) ast->expr->accept(this);
        std::cout << ")";
    }
}


//
// AstAddressOfExpr
void PrinterVisitor::visit(AstAddressOfExpr * ast)
{
    std::cout << "&";
    if (ast->id) ast->id->accept(this);
}


//
// AstDereferenceExpr
void PrinterVisitor::visit(AstDereferenceExpr * ast)
{
    std::cout << "*";
    if (ast->expr) ast->expr->accept(this);
}


//
// AstIdentExpr
void PrinterVisitor::visit(AstIdentExpr * ast)
{
    if (ast->token) std::cout << ast->token->lexeme();
}


//
// AstLiteralExpr
void PrinterVisitor::visit(AstLiteralExpr * ast)
{
    if (ast->token) {
        if (ast->token->type() == TokenType::StringLiteral) {
            std::cout << '"';
            for (auto ch : ast->token->lexeme()) {
                if      (ch == '\n') std::cout << "\\n";
                else if (ch == '\r') std::cout << "\\r";
                else if (ch == '\t') std::cout << "\\t";
                else if (ch == '\\') std::cout << "\\\\";
                else if (ch == '"')  std::cout << "\\\"";
                else                 std::cout << ch;
            }
            std::cout << '"';
        } else {
            std::cout << ast->token->lexeme();
        }
    }
}


//
// AstBinaryExpr
void PrinterVisitor::visit(AstBinaryExpr * ast)
{
    if (ast->lhs) ast->lhs->accept(this);
    if (ast->token) std::cout << " " << ast->token->lexeme() << " ";
    if (ast->rhs) ast->rhs->accept(this);
}


//
// AstCallExpr
void PrinterVisitor::visit(AstCallExpr * ast)
{
    if (ast->id) ast->id->accept(this);
    std::cout << "(";
    if (ast->args) ast->args->accept(this);
    std::cout << ")";
}


//
// AstAttributeList
void PrinterVisitor::visit(AstAttributeList * ast)
{
    std::cout << indent() << '[';
    bool first = true;
    for (auto & attr : ast->attribs) {
        if (first) first = false;
        else std::cout << ", ";
        attr->accept(this);
    }
    std::cout << "] _" << std::endl;
}


//
// AstAttribute
void PrinterVisitor::visit(AstAttribute * ast)
{
    if (ast->id) ast->id->accept(this);
    if (ast->params != nullptr) {
        if (ast->params->params.size() == 1) {
            std::cout << " = ";
            ast->params->accept(this);
        } else {
            std::cout << "(";
            ast->params->accept(this);
            std::cout << ")";
        }
    }
}


//
// AstAttribParamList
void PrinterVisitor::visit(AstAttribParamList * ast)
{
    bool first = true;
    for (auto & p : ast->params) {
        if (first) first = false;
        else std::cout << ", ";
        p->accept(this);
    }
}


//
// AstTypeExpr
void PrinterVisitor::visit(AstTypeExpr * ast)
{
    if (ast->token) std::cout << ast->token->lexeme();
    if (ast->level) {
        int i = ast->level;
        while (i--) std::cout << " PTR";
    }
}


//
// AstFuncParamList
void PrinterVisitor::visit(AstFuncParamList * ast)
{
    bool first = true;
    for (auto & param : ast->params) {
        if (first) first = false;
        else std::cout << ", ";
        param->accept(this);
    }
}


//
// AstFuncParam
void PrinterVisitor::visit(AstFuncParam * ast)
{
    if (ast->id) ast->id->accept(this);
    std::cout << " AS ";
    if (ast->typeExpr) ast->typeExpr->accept(this);
}


//
// AstFuncArgList
void PrinterVisitor::visit(AstFuncArgList * ast)
{
    bool first = true;
    for (auto & arg : ast->args) {
        if (first) first = false;
        else std::cout << ", ";
        arg->accept(this);
    }
}

