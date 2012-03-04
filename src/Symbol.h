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
    
    /**
     * Symbol represents a single instance of a type (variables, functions, ...)
     */
    struct Symbol : NonCopyable
    {
        // create new symbol
        Symbol(const string & id, const shared_ptr<Type> & type, AstDeclaration * decl = nullptr, AstDeclaration * impl = nullptr);
        
        // clean up
        ~Symbol();
        
        // get id
        const string & id() const { return m_id; }
        
        // get type
        shared_ptr<Type> type() const { return m_type; }
        
        // set type
        void type(const shared_ptr<Type> & type) { m_type = type; }
        
        // get declaration ast node
        AstDeclaration * decl() const { return m_decl; }
        
        // set declaration ast node
        void decl(AstDeclaration * decl) { m_decl = decl; }
        
        // get implementation ast node
        AstDeclaration * impl() const { return m_impl; }
        
        // set implementation ast node
        void impl(AstDeclaration * impl) { m_impl = impl; }
        
        // allocate objects from the pool
        void * operator new(size_t);
        void operator delete(void *);
        
        // llvm argument
        llvm::Value * value;
        
    private:
        // id
        string m_id;
        // symbol type
        shared_ptr<Type> m_type;
        // symbol declaration (ast node)
        AstDeclaration * m_decl;
        // implementation ( functions may have seperate declaration )
        AstDeclaration * m_impl;
    };
    
}