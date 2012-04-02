//
//  Symbol.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 29/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "Symbol.h"
#include "SymbolTable.h"

using namespace lbc;

// Tokens memory pool
static boost::pool<> _pool(sizeof(Symbol));


// allocate
void * Symbol::operator new(size_t)
{
    return _pool.malloc();
}


// release
void Symbol::operator delete(void * addr)
{
    _pool.free(addr);
}



/**
 * Create new Symbol object
 */
Symbol::Symbol(const string & id, Type * type, AstDeclaration * decl, AstDeclaration * impl, SymbolTable * scope)
: m_id(id), m_type(type), m_decl(decl), m_impl(impl), value(nullptr), m_scope(scope)
{
    
}



/**
 * clean up
 */
Symbol::~Symbol() {}
