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
        Emitter(Context & ctx);
        
        // destroy
        virtual ~Emitter();
        
        // add module. This takes ownership of the module!
        void add(llvm::Module * module);
        
        // generate the output
        void generate();
        
    private:
        
        // tools
        enum class Tool {
            LlvmOpt,            // LLVM optimizer
            LlvmLlc,            // LLVM assembler (native code generator)
            Ld                  // System linker
        };
        
        // compiler state context
        Context & m_ctx;
        
        // emit targets
        void emitExecutable();
        void emitLibrary();
        void emitObjOrAsm();
        void emitLlvm();
        
        // get tool path
        string getTool(Tool tool);
        
        // llvm modules
        vector<unique_ptr<llvm::Module>> m_modules;
    };
    
}
