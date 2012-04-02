//
//  SourceString.h
//  LightBasic
//
//  Created by Albert on 20/03/2011.
//  Copyright 2011 LightBasic Development Team. All rights reserved.
//
#pragma once

#include "Source.h"

namespace lbc {
    
    /**
     * Source from a string
     * Useful for tests...
     */
    class SourceString : public Source
    {
        public:
        
        /// Create instance
        SourceString(const string & content, const string & name = "");
        
        /// cleanup
        virtual ~SourceString();
    };
    
}
