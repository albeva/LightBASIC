//
//  Symbol.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 29/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "Symbol.h"
#include "SymbolTable.h"
#include "MemoryPool.h"

using namespace lbc;

// Tokens memory pool
static MemoryPool<Symbol> _pool;

// allocate
void * Symbol::operator new(size_t)
{
    return _pool.allocate();
}


// release
void Symbol::operator delete(void * addr)
{
    _pool.deallocate(addr);
}



/**
 * Create new Symbol object
 */
Symbol::Symbol(const std::string & id, Type * type, AstDeclaration * decl, AstDeclaration * impl, SymbolTable * scope)
:   value(nullptr),
    m_id(id),
    m_type(type),
    m_scope(scope),
    m_decl(decl),
    m_impl(impl)
{
}



/**
 * clean up
 */
Symbol::~Symbol() {}
