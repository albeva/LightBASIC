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
    struct PrinterVisitor : RecursiveAstVisitor
    {
        PrinterVisitor();
        
        // indent by number of spaces
        string ndent(int change);

        // AstAssignStmt
        void visit(AstAssignStmt * ast);
        // AstReturnStmt
        void visit(AstReturnStmt * ast);
        // AstCallStmt
        void visit(AstCallStmt * ast);
        // AstIfStmt
        void visit(AstIfStmt * ast);
        // AstForStmt
        void visit(AstForStmt * ast);
        // AstDeclaration
        void visit(AstDeclaration * ast);
        // AstVarDecl
        void visit(AstVarDecl * ast);
        // AstFunctionDecl
        void visit(AstFunctionDecl * ast);
        // AstFuncSignature
        void visit(AstFuncSignature * ast);
        // AstFunctionStmt
        void visit(AstFunctionStmt * ast);
        // AstCastExpr
        void visit(AstCastExpr * ast);
        // AstAddressOfExpr
        void visit(AstAddressOfExpr * ast);
        // AstDereferenceExpr
        void visit(AstDereferenceExpr * ast);        
        // AstIdentExpr
        void visit(AstIdentExpr * ast);
        // AstLiteralExpr
        void visit(AstLiteralExpr * ast);
        // AstBinaryExpr
        void visit(AstBinaryExpr * ast);
        // AstCallExpr
        void visit(AstCallExpr * ast);
        // AstAttributeList
        void visit(AstAttributeList * ast);
        // AstAttribParamList
        void visit(AstAttribParamList * ast);
        // AstAttribute
        void visit(AstAttribute * ast);
        // AstTypeExpr
        void visit(AstTypeExpr * ast);
        // AstFuncParamList
        void visit(AstFuncParamList * ast);
        // AstFuncParam
        void visit(AstFuncParam * ast);
        // AstFuncArgList
        void visit(AstFuncArgList * ast);
        
    private:
        int m_indent;
        bool m_elseIf;
        bool m_doInline;
        string indent(int change = 0);
    };

}