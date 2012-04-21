//
//  Context.h
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

#include "Path.h"
#include <llvm/ADT/Triple.h>
#include <llvm/Support/SourceMgr.h>

namespace lbc {
    
    // resource container
    typedef std::vector<std::string> ResourceContainer;
    
    // Types of resources
    enum class ResourceType {
        GlobalPath,
        Source,
        SourcePath,
        Library,
        LibraryPath,
        _Count
    };
    
    // Optimization level
    enum class OptimizationLevel {
        O0,
        O1,
        O2,
        O3
    };
    static inline std::string getOptimizationLevel(OptimizationLevel level) {
        if      (level == OptimizationLevel::O0) return "-O0";
        else if (level == OptimizationLevel::O1) return "-O1";
        else if (level == OptimizationLevel::O2) return "-O2";
        else return "-O3";
    }
    
    // Emit mode
    enum class EmitType {
        Executable,
        Library,
        Object,
        Asm,
        Llvm
    };
    static inline std::string getEmitType(EmitType emit) {
        if      (emit == EmitType::Executable)  return "";
        else if (emit == EmitType::Library)     return "";
        else if (emit == EmitType::Object)      return "-c";
        else if (emit == EmitType::Asm)         return "-S";
        else return "-llvm";
    }

    /**
     * Porpuse of this class is to gather context information
     * about the compilation. Include folders, files to compile,
     * compile options, etc.
     */
    class Context : NonCopyable
    {
    public:
        
        // get singleton object.
        static Context & getGlobalContext();
        
        // Initialize the context after all
        // options are set. Returns true if successful, false otherwise.
        bool initialize();
        
        // create new global context
        static Context * create() { return new Context(); }
        
        // Add resource
        Context & add(const Path & path, ResourceType type);
        
        // get resources
        const ResourceContainer & get(ResourceType type) const;
        
        // output path
        const Path & output() const;
        Context & output(const Path & path);
        
        // get / set compiler path
        const Path & compiler() const { return m_compiler; }
        Context & compiler(const Path & path) { m_compiler = path; return *this; }
        
        // verbose
        bool verbose() const { return m_verbose; }
        Context & verbose(bool verbose) { m_verbose = verbose; return *this; }
        
        // optimization level
        OptimizationLevel opt() const { return m_optLevel; }
        Context & opt(OptimizationLevel level) { m_optLevel = level; return *this; }
        
        // emit mode
        EmitType emit() const { return m_emit; }
        Context & emit(EmitType emit) { m_emit = emit; return *this; }
        
        // get / set triple
        llvm::Triple & triple() { return m_triple; }
        Context & triple(const llvm::Triple & t) { m_triple = t; return *this; }
        
        // get src mgr
        llvm::SourceMgr & sourceMrg() { return m_sourceMgr; }
        
        // generate a string containing all current options
        std::string toString() const;
        
    private:
        
        // create
        Context();
        
        /// Resolve directory path
        Path resolveDir(const Path & path) const;
        
        /// resolve resource path against the container
        Path resolveFile(const Path & path, ResourceType type) const;
        
        /// array that contains the resources
        ResourceContainer m_resources[(int)ResourceType::_Count];
        
        // output path including filename
        // make it mutable for caching
        mutable Path m_output;
        
        // compiler path
        Path m_compiler;
        
        // verbose build?
        bool m_verbose;
        
        // build for architecture
        llvm::Triple        m_triple;
        llvm::SourceMgr     m_sourceMgr;
        OptimizationLevel   m_optLevel;
        EmitType            m_emit;
    };

} // lbc namespace

