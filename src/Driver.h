//
//  Driver.h
//  LightBASIC
//
//  Created by Albert Varaksin on 21/04/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

namespace lbc {

/**
 * the context
 */
class Context;


/**
 * Driver the compilation process. Assume no logic.
 * Simply run the steps and abort if errors encountered
 */
class Driver : NonCopyable
{
public:
    /**
     * Create the driver
     */
    Driver(Context & ctx);
    
    /**
     * Drive the compilation process
     */
    void drive();
    
private:
    Context & m_ctx;
};
    
    
} // ~lbc namespace
