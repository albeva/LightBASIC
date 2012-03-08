//
//  Emitter.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 08/03/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "Emitter.h"
#include "Context.h"

#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/IRBuilder.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>

using namespace lbc;


/**
 * Create new Emitter instance
 */
Emitter::Emitter(const shared_ptr<Context> & ctx) : m_context(ctx)
{}


/**
 * clean up
 */
Emitter::~Emitter()
{}


/**
 * add module to the emitter's list
 */
void Emitter::add(llvm::Module * module)
{
	m_modules.push_back(module);
}


/**
 * generate the results
 */
void Emitter::generate()
{
	
}

