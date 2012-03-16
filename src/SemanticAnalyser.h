//
//  SemanticAnalyser.h
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

#include "Ast.h"
#include "RecursiveAstVisitor.h"

namespace lbc {
    
    // forward reference
    class SymbolTable;
    class Symbol;
    class Type;

    /**
     * Perform semantic analyses on the Ast tree
     * and build symbol table along the way
     */
    struct SemanticAnalyser : RecursiveAstVisitor
    {
        SemanticAnalyser();
        
        // process expression
        // this CAN replace the current ast node
        // with a new one!
        void expression(unique_ptr<AstExpression> & ast);
        
        // coerce expression to a type if needed
        void coerce(unique_ptr<AstExpression> & ast, Type * type);
        
        // AstDeclList
        virtual void visit(AstProgram * ast);
        
        // AstFunctionDecl
        virtual void visit(AstFunctionDecl * ast);
        
        // AstFuncSignature
        virtual void visit(AstFuncSignature * ast);
        
        // AstFuncParam
        virtual void visit(AstFuncParam * ast);
        
        // AstTypeExpr
        virtual void visit(AstTypeExpr * ast);
        
        // AstVarDecl
        virtual void visit(AstVarDecl * ast);
        
        // AstAssignStmt
        virtual void visit(AstAssignStmt * ast);
        
        // AstLiteralExpr
        virtual void visit(AstLiteralExpr * ast);
        
        // AstCallStmt
        virtual void visit(AstCallStmt * ast);
        
        // AstAddressOfExpr
        virtual void visit(AstAddressOfExpr * ast);
        
        // AstDereferenceExpr
        virtual void visit(AstDereferenceExpr * ast);
        
        // AstBinaryExpr
        virtual void visit(AstBinaryExpr * ast);
        
        // AstCallExpr
        virtual void visit(AstCallExpr * ast);
        
        // AstIdentExpr
        virtual void visit(AstIdentExpr * ast);
        
        // AstFunctionStmt
        virtual void visit(AstFunctionStmt * ast);
        
        // AstReturnStmt
        virtual void visit(AstReturnStmt * ast);
        
        // AstAttribute
        virtual void visit(AstAttribute * ast);
        
    private:
        // cyrrent scope backeed by symbol table
        SymbolTable * m_table;
        // cyrrent symbol
        Symbol * m_symbol;
        // current type
        Type * m_type;
        // coercion type
        Type * m_coerceType;
    };

}
