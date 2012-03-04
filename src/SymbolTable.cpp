//
//  SymbolTable.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "SymbolTable.h"
#include "Symbol.h"
using namespace lbc;


/**
 * Create the symbol table
 */
SymbolTable::SymbolTable(SymbolTable * parent) : m_parent(parent)
{
}


/**
 * clean up the symbol table
 */
SymbolTable::~SymbolTable()
{
    for (auto iter : m_symbols) delete iter.second;
}


/**
 * add symbol to the table. Overwrite if exists.
 */
void SymbolTable::add(const string & id, Symbol * sym)
{
    m_symbols[id] = sym;
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
    // symbol exists?
    auto iter = m_symbols.find(id);
    if (iter != m_symbols.end()) return iter->second;
    
    // get from the parent?
    // DON'T cache because symbol table owns the symbols
    // so when cleaning up there is no way to know to wich table
    // symbol belongs. Options:
    // - seperate hashmap for cached symbols. Simple, but reduce perfomance and increase memory usage
    // - make symbols shared_ptr - decrease perfomance and increase memory usage
    // - have symbols to know their table - simple. probably will go for this. later
    // - some other option?
    if (recursive && m_parent != nullptr) return m_parent->get(id, true);
    
    // nothing found.
    return nullptr;
}
