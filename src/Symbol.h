//
//  Symbol.h
//  LightBASIC
//
//  Created by Albert Varaksin on 29/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

namespace llvm {
    class Value;
}

namespace lbc {
    
    // forward declaration
    class Type;
    class AstDeclaration;
    class SymbolTable;
    
    /**
     * Symbol represents a single instance of a type (variables, functions, ...)
     */
    struct Symbol : NonCopyable
    {
        // create new symbol
        Symbol(const string & id, Type * type = nullptr,
               AstDeclaration * decl = nullptr, AstDeclaration * impl = nullptr,
               SymbolTable * scope = nullptr);
        
        // clean up
        ~Symbol();
        
        // get id
        const string & id() const { return m_id; }
        
        // alias
        const string & alias() const { return m_alias.length() ? m_alias : m_id; }
        void alias(const string & alias) { m_alias = alias; }
        
        // type
        Type * type() const { return m_type; }
        void type(Type * type) { m_type = type; }
        
        // declaration ast node
        AstDeclaration * decl() const { return m_decl; }
        void decl(AstDeclaration * decl) { m_decl = decl; }
        
        // implementation ast node
        AstDeclaration * impl() const { return m_impl; }
        void impl(AstDeclaration * impl) { m_impl = impl; }
        
        // scope
        SymbolTable * scope() const { return m_scope; }
        void scope(SymbolTable * scope) { m_scope = scope; }
        
        // allocate objects from the pool
        void * operator new(size_t);
        void operator delete(void *);
        
        // llvm argument
        llvm::Value * value;
        
    private:
        // id
        string m_id, m_alias;
        // symbol type
        Type * m_type;
        // symbol table this belongs to
        SymbolTable * m_scope;
        // symbol declaration (ast node)
        AstDeclaration * m_decl;
        // implementation ( functions may have seperate declaration )
        AstDeclaration * m_impl;
    };
    
}
