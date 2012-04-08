//
//  SymbolTable.h
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

namespace lbc {

    // type information
    class Symbol;
    
    /**
     * Keep information about defined symbols (variables, functions, ...)
     */
    class SymbolTable : NonCopyable
    {
    public:
        
        // the iterator
        typedef std::unordered_map<std::string, Symbol *>::iterator iterator;
        
        // create new symbol table. Providing parent will inherit the scope
        SymbolTable(SymbolTable * parent = nullptr);
        
        // 
        ~SymbolTable();
        
        // get parent symbol table
        SymbolTable * parent() const { return m_parent; }
        
        // add new type
        void add(Symbol * type);
        
        // exists?
        bool exists(const std::string & id, bool recursive = false);
        
        // get
        Symbol * get(const std::string & id, bool recursive = true);
        
        // begin
        iterator begin() { return m_symbols.begin(); }
        
        // end
        iterator end() { return m_symbols.end(); }
        
    private:
        // parent table
        SymbolTable * m_parent;
        
        // symbols
        std::unordered_map<std::string, Symbol *> m_symbols;
    };

}
