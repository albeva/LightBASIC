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
    class Type;
    class Context;
    enum class TokenType : int;

    /**
     * Generate the llvm structure
     */
    class IrBuilder : public RecursiveAstVisitor
    {
    public:
        using RecursiveAstVisitor::visit;
        
        // create
        IrBuilder(Context & ctx);
        
        // get the generated module
        llvm::Module * getModule() const { return m_module; }
        
        // AstDeclList
        virtual void visit(AstProgram * ast);
        
        // AstStmtList
        virtual void visit(AstStmtList * ast);
        
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
        
        // AstCastExpr
        virtual void visit(AstCastExpr * ast);
        
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
        
        // AstCallStmt
        virtual void visit(AstCallStmt * ast);
        
        // AstIfStmt
        virtual void visit(AstIfStmt * ast);
        
        // AstForStmt
        virtual void visit(AstForStmt * ast);
        
        
        // emit binary instruction
        llvm::Value * emitBinaryExpr(llvm::Value * left, llvm::Value * right, TokenType op, Type * type);
        
        // emit cast instruction
        llvm::Value * emitCastExpr(llvm::Value * value, Type * from, Type * to);
        
    private:
        llvm::Module * m_module;
        llvm::Function * m_function;
        llvm::BasicBlock * m_block, * m_edgeBlock;
        llvm::Value * m_value;
        SymbolTable * m_table;
        bool m_isElseIf;
        std::string m_lastId;
        std::unordered_map<std::string, llvm::Value *> m_stringLiterals;
        Context & m_ctx;
    };
    
}

