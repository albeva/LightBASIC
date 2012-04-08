//
//  PrinterVisitor.h
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

#include "Ast.h"
#include "RecursiveAstVisitor.h"

namespace lbc {

    /**
     * print out the Ast tree
     */
    class PrinterVisitor : public RecursiveAstVisitor
    {
    public:
        using RecursiveAstVisitor::visit;
        
        // create
        PrinterVisitor();

        // AstAssignStmt
        virtual void visit(AstAssignStmt * ast);
        // AstReturnStmt
        virtual void visit(AstReturnStmt * ast);
        // AstCallStmt
        virtual void visit(AstCallStmt * ast);
        // AstIfStmt
        virtual void visit(AstIfStmt * ast);
        // AstForStmt
        virtual void visit(AstForStmt * ast);
        // AstDeclaration
        virtual void visit(AstDeclaration * ast);
        // AstVarDecl
        virtual void visit(AstVarDecl * ast);
        // AstFunctionDecl
        virtual void visit(AstFunctionDecl * ast);
        // AstFuncSignature
        virtual void visit(AstFuncSignature * ast);
        // AstFunctionStmt
        virtual void visit(AstFunctionStmt * ast);
        // AstCastExpr
        virtual void visit(AstCastExpr * ast);
        // AstAddressOfExpr
        virtual void visit(AstAddressOfExpr * ast);
        // AstDereferenceExpr
        virtual void visit(AstDereferenceExpr * ast);        
        // AstIdentExpr
        virtual void visit(AstIdentExpr * ast);
        // AstLiteralExpr
        virtual void visit(AstLiteralExpr * ast);
        // AstBinaryExpr
        virtual void visit(AstBinaryExpr * ast);
        // AstCallExpr
        virtual void visit(AstCallExpr * ast);
        // AstAttributeList
        virtual void visit(AstAttributeList * ast);
        // AstAttribParamList
        virtual void visit(AstAttribParamList * ast);
        // AstAttribute
        virtual void visit(AstAttribute * ast);
        // AstTypeExpr
        virtual void visit(AstTypeExpr * ast);
        // AstFuncParamList
        virtual void visit(AstFuncParamList * ast);
        // AstFuncParam
        virtual void visit(AstFuncParam * ast);
        // AstFuncArgList
        virtual void visit(AstFuncArgList * ast);
        
    private:
        int m_indent;
        bool m_elseIf;
        bool m_doInline;
        std::string indent(int change = 0);
    };

}
