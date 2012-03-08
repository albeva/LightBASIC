//
//  IrBuilder.h
//  LightBASIC
//
//  Created by Albert Varaksin on 03/03/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once
#include "RecursiveAstVisitor.h"

// forward declare llvm classes
namespace llvm {
    // the module
    class Module;
    class Type;
    class Function;
    class Value;
    class BasicBlock;
}

namespace lbc {
    
    // the symbol table
    class SymbolTable;

    /**
     * Generate the llvm structure
     */
    struct IrBuilder : RecursiveAstVisitor
    {
		// create
        IrBuilder();
		
		// reset the ir builder
		void reset();
        
        // get the generated module
        llvm::Module * getModule() const { return m_module; }
        
        // AstDeclList
        virtual void visit(AstProgram * ast);
        
        // AstFunctionDecl
        virtual void visit(AstFunctionDecl * ast);
        
        // AstFuncSignature
        virtual void visit(AstFuncSignature * ast);
        
        // AstVarDecl
        virtual void visit(AstVarDecl * ast);
        
        // AstAssignStmt
        virtual void visit(AstAssignStmt * ast);
        
        // AstLiteralExpr
        virtual void visit(AstLiteralExpr * ast);
        
        // AstCallStmt
        virtual void visit(AstCallStmt * ast);
        
        // AstCallExpr
        virtual void visit(AstCallExpr * ast);
        
        // AstIdentExpr
        virtual void visit(AstIdentExpr * ast);
        
        // AstFunctionStmt
        virtual void visit(AstFunctionStmt * ast);
        
        // AstReturnStmt
        virtual void visit(AstReturnStmt * ast);
        
    private:
        llvm::Module * m_module;
        llvm::Function * m_function;
        llvm::BasicBlock * m_block;
        llvm::Value * m_value;
        SymbolTable * m_table;
    };
    
}

