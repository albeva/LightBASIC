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
        throw Exception("Could not open '" + getName() + "'");
    }

    // get lenght
    stream.seekg(0, ios::end);
    size_t size = (size_t)stream.tellg();
    stream.seekg (0, ios::beg);
    
    // the buffer
    m_data = new char[size + 1];
    stream.read(m_data, size);
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
