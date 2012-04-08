//
//  Source.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "Source.h"
#include "SourceLocation.h"
using namespace lbc;

// Get line as a string
// line numbers start from 1
std::string Source::getLine(unsigned int line)
{
    const CharT * start = getLinePtr(line);
    if (start == NULL) return "";
    const CharT * iter = start;
    while (*iter != '\n' && *iter != '\r' && *iter != '\0') iter++;
    return std::string(start, iter);
}


// get string
// Should this be *safe* but slow lookup? Theoretically
// location should be guaranteed to be valid within the source...
std::string Source::getString(const SourceLocation & loc)
{
    // get line
    const CharT * start = getLinePtr(loc.getLine());
    if (start == NULL) return "";
    
    // search for start
    int col = loc.getColumn();
    while (--col) {
        if (*start == '\0' || *start == '\n' || *start == '\r') return "";
        start++;
    }
    
    // search for end
    int len = loc.getLength();
    const CharT * iter = start;
    while(len--) {
        if (*iter == '\0' || *iter == '\n' || *iter == '\r') return "";
        iter++;
    }
    
    // done
    return std::string(start, iter);
}


// get line ptr
const Source::CharT * Source::getLinePtr(unsigned int line)
{
    line--;
    
    // if line is not in the cache then create an iterator and 
    // go through the code until end and try again
    if (line >= m_lineCache.size()) {
        size_t l = m_lineCache.size();
        const_iterator iter(l > 0 ? (CharT *)(m_lineCache.back() + (char *)m_data): m_data, this);
        while(line <= m_lineCache.size() && *iter != '\0') iter++;
        if (line >= m_lineCache.size()) return NULL;
    }
    
    return (CharT *)(m_lineCache[line] + (char *)m_data);
}

