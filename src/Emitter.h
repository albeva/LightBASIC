//
//  Emitter.h
//  LightBASIC
//
//  Created by Albert Varaksin on 08/03/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

namespace llvm {
	class Module;
}

namespace lbc {
	
	// the context
	class Context;
	
	/**
	 * This class deals with combingin the llvm ir's and generating the
	 * output executable or a library
	 */
	class Emitter : NonCopyable {
	public:
		
		// create
		Emitter(const shared_ptr<Context> & ctx);
		
		// destroy
		virtual ~Emitter();
		
		// add module. This takes ownership of the module!
		void add(llvm::Module * module);
		
		// generate the output
		void generate();
		
	private:
		// compiler state context
		shared_ptr<Context> m_context;
		
		// llvm modules
		vector<unique_ptr<llvm::Module>> m_modules;
	};
	
}
