//
//  SourceString.cpp
//  LightBasic
//
//  Created by Albert on 20/03/2011.
//  Copyright 2011 LightBasic Development Team. All rights reserved.
//

#include "SourceString.h"
using namespace lbc;


/// Create instance
SourceString::SourceString(const string & content, const string & name) : Source(name)
{
    m_data = new CharT[content.size() + 1];
    copy(content.begin(), content.end(), m_data);
    m_data[content.size()] = '\0';
}


/// cleanup
SourceString::~SourceString()
{
    if (m_data != nullptr) {
        delete m_data;
    }
}
