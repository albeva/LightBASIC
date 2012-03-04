//
//  SourceFile.h
//  LightBasic
//
//  Created by Albert on 20/03/2011.
//  Copyright 2011 LightBasic Development Team. All rights reserved.
//
#pragma once

#include "Source.h"

namespace lbc
{
    
    /**
     * Read source from file
     */
    class SourceFile : public Source
    {
        public:
        
        /// Create SourceFile instance
        SourceFile(const FS::path & path);
        
        // destructor
        virtual ~SourceFile();
    };
    
}