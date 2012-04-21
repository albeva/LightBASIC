//
//  Driver.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 21/04/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include <iostream>
#include "Driver.h"
#include "Context.h"
#include "Parser.h"
#include "Ast.h"
#include "PrinterVisitor.h"
#include "SemanticAnalyser.h"
#include "IrBuilder.h"
#include "Emitter.h"

using namespace lbc;


/**
 * Create the driver
 */
Driver::Driver(Context & ctx) : m_ctx(ctx)
{
}


/**
 * Drive the compilation process
 */
void Driver::drive()
{
    // Initialize the Context
    m_ctx.initialize();
    
    // create the parser
    Parser parser(m_ctx);
    // semantic analyser
    SemanticAnalyser semantic(m_ctx);
    // IR builder
    IrBuilder irBuilder(m_ctx);
    // create output emitter
    Emitter emitter(m_ctx);
    // process the files
    bool successful = true;
    for (auto & file : m_ctx.get(ResourceType::Source)) {
        auto ast = parser.parse(file);
        if (ast) {
            // analyse
            ast->accept(&semantic);
            
            // generate llvm code
            ast->accept(&irBuilder);
            if (auto module = irBuilder.getModule()) {
                emitter.add(module);
            }
            
            // cleanup
            delete ast;
        } else {
            successful = false;
        }
    }
    // generate the output
    if (successful) {
        emitter.generate();
    }
}
