//
//  Token.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "Token.h"
#include "MemoryPool.h"
using namespace lbc;

// define array of token names
static std::string _tokenNames[] = {
    #define IMPL_TOKENS(ID, NAME) NAME, 
    ALL_TOKENS(IMPL_TOKENS)
    #undef IMPL_TOKENS
};

// keyword lookup
static std::unordered_map<std::string, TokenType> _tokenTypes = {
    #define IMPL_TOKENS(ID, NAME) {NAME, TokenType::ID},
    ALL_TOKENS(IMPL_TOKENS)
    #undef IMPL_TOKENS   
};


/**
 * Get token type name
 */
const std::string & Token::getTokenName(TokenType type)
{
    return _tokenNames[(int)type];
}


/**
 * get token type based on string
 */
TokenType Token::getTokenType(const std::string & id, TokenType def)
{
    auto iter = _tokenTypes.find(id);
    if (iter != _tokenTypes.end()) return iter->second;
    return def;
}


// Tokens memory pool
static MemoryPool<Token> _pool;


// allocate
void * Token::operator new(size_t)
{
    return _pool.allocate();
}


// release
void Token::operator delete(void * addr)
{
    _pool.deallocate(addr);
}

