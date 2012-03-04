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
	struct SymbolTable : NonCopyable
	{
		// create new symbol table. Providing parent will inherit the scope
		SymbolTable(SymbolTable * parent = nullptr) : m_parent(parent) {}
		
		// get parent symbol table
		SymbolTable * parent() const { return m_parent; }
		
		// add new type
		void add(const string & id, Symbol * type);
		
		// exists?
		bool exists(const string & id, bool recursive = false);
		
		// get
		Symbol * get(const string & id, bool recursive = true);
		
		// allocate objects from the pool
		void * operator new(size_t);
		void operator delete(void *);
		
	private:
		// parent table
		SymbolTable * m_parent;
		
		// symbols
		unordered_map<string, Symbol *> m_symbols;
	};

}