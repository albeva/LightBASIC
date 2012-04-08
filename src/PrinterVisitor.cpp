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
PrinterVisitor::PrinterVisitor()
:   m_indent(0),
    m_elseIf(false),
    m_doInline(false)
{}


//
// indent by number of spaces
std::string PrinterVisitor::indent(int change)
{
    m_indent += change;
    return std::string((size_t)(m_indent * 4), ' ');
}


//
// AstAssignStmt
void PrinterVisitor::visit(AstAssignStmt * ast)
{
    // indent
    if (!m_doInline) std::cout << indent();
    
    // left = right
    if (ast->left) ast->left->accept(this);
    std::cout << " = ";
    if (ast->right) ast->right->accept(this);
    
    // \n
    if (!m_doInline) std::cout << std::endl;
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
// AstIfStmt
void PrinterVisitor::visit(AstIfStmt * ast)
{
    if (!m_elseIf) std::cout << indent();
    SCOPED_GUARD(m_elseIf);
    m_elseIf = false;
    
    std::cout << "IF ";
    if (ast->expr) ast->expr->accept(this);
    std::cout << " THEN\n";
    m_indent++;
    if (ast->trueBlock) ast->trueBlock->accept(this);
    m_indent--;
    std::cout << indent();
    if (ast->falseBlock) {
        if (ast->falseBlock->is(Ast::IfStmt)) {
            std::cout << "ELSE ";
            m_elseIf = true;
            ast->falseBlock->accept(this);
            return;
        } else {
            std::cout << "ELSE\n";
            m_indent++;
            ast->falseBlock->accept(this);
            m_indent--;
            std::cout << indent();
        }
    }
    std::cout << "END IF\n";
}


//
// AstForStmt
void PrinterVisitor::visit(AstForStmt * ast)
{   
    // for
    std::cout << indent() << "FOR ";
    
    // lhs = expr
    if (ast->stmt) {
        SCOPED_GUARD(m_doInline);
        m_doInline = true;
        ast->stmt->accept(this);
    }
    
    // to expr
    std::cout << " TO ";
    if (ast->end) ast->end->accept(this);
    
    // [ step expr ]
    if (ast->step) {
        std::cout << " STEP ";
        ast->step->accept(this);
    }
    
    // \n
    std::cout << '\n';
    if (ast->block) {
        SCOPED_GUARD(m_indent);
        m_indent++;
        ast->block->accept(this);
    }
    // Next
    std::cout << indent() << "NEXT\n";
}


//
// AstDeclaration
void PrinterVisitor::visit(AstDeclaration *)
{
    std::cout << "SHOULD NEVER GET HERE!";
}


//
// AstVarDecl
void PrinterVisitor::visit(AstVarDecl * ast)
{
    // indent
    if (!m_doInline) std::cout << indent();
    // DIM
    if (ast->typeExpr) {
        std::cout << "DIM ";
        if (ast->id) ast->id->accept(this);
        std::cout << " AS ";
        if (ast->typeExpr) ast->typeExpr->accept(this);
        if (ast->expr) {
            std::cout << " = ";
            ast->expr->accept(this);
        }
    }
    // VAR
    else {
        std::cout << "VAR ";
        if (ast->id) ast->id->accept(this);
        if (ast->expr) {
            std::cout << " = ";
            ast->expr->accept(this);
        }
    }
    // \n
    if (!m_doInline) std::cout << std::endl;
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
    if (ast->typeExpr) {
        std::cout << "FUNCTION ";
    } else {
        std::cout << "SUB ";
    }
    if (ast->id) ast->id->accept(this);
    if (ast->vararg || (ast->params && ast->params->params.size())) {
        std::cout << "(";
        if (ast->params) ast->params->accept(this);
        if (ast->vararg) {
            if (ast->params && ast->params->params.size()) std::cout << ", ";
            std::cout << "...";
        }
        std::cout << ")";
    }
    if (ast->typeExpr) {
        std::cout << " AS ";
        ast->typeExpr->accept(this);
    }
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
    std::cout << indent() << "END ";
    if (ast->signature && ast->signature->typeExpr) {
        std::cout << "FUNCTION" << std::endl;
    } else {
        std::cout << "SUB" << std::endl;
    }
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

