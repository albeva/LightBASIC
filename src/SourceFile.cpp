//
//  SourceFile.cpp
//  LightBasic
//
//  Created by Albert on 20/03/2011.
//  Copyright 2011 LightBasic Development Team. All rights reserved.
//

#include "SourceFile.h"
using namespace lbc;


/**
 * Create instance of the SourceFile
 */
SourceFile::SourceFile(const FS::path & path) : Source(path.string())
{
    ifstream stream;
    stream.open(getName().c_str(), ios::in | ios::binary);
    if (!stream.good()) {
        THROW_EXCEPTION("Could not open '" + getName() + "'");
    }

    // get lenght
    stream.seekg(0, ios::end);
    streamsize size = (streamsize)stream.tellg();
    stream.seekg (0, ios::beg);
    
    // the buffer
    m_data = new CharT[size + 1];
    stream.read((char *)m_data, size);
    stream.close();
    m_data[size] = '\0';
}


/**
 * cleanup
 */
SourceFile::~SourceFile()
{
    if (m_data != nullptr) {
        delete m_data;
    }
}
