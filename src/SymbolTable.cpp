//
//  SymbolTable.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "SymbolTable.h"
using namespace lbc;

// Tokens memory pool
static boost::pool<> _pool(sizeof(SymbolTable));


// allocate
void * SymbolTable::operator new(size_t)
{
	return _pool.malloc();
}


// release
void SymbolTable::operator delete(void * addr)
{
	_pool.free(addr);
}


/**
 * add symbol to the table. Overwrite if exists.
 */
void SymbolTable::add(const string & id, Symbol * type)
{
	m_symbols[id] = type;
}


/**
 * does teh symbol exist? If recursive then will check parent scope
 * and its parent. all the way to the root level
 */
bool SymbolTable::exists(const string & id, bool recursive)
{
	if (m_symbols.find(id) != m_symbols.end()) return true;
	return recursive && m_parent != nullptr && m_parent->exists(id, true);
}


/**
 * get symbol. will cache the result in local scope if found.
 */
Symbol * SymbolTable::get(const string & id, bool recursive)
{
	auto iter = m_symbols.find(id);
	if (iter != m_symbols.end()) return iter->second;
	
	if (recursive && m_parent != nullptr) {
		auto sym = m_parent->get(id, true);
		if (sym != nullptr) m_symbols[id] = sym;
		return sym;
	}
	
	return nullptr;
}
