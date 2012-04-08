//
//  Context.h
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

namespace lbc {
    
    // resource container
    typedef std::vector<FS::path> ResourceContainer;
    
    // Types of resources
    enum class ResourceType {
        GlobalPath,
        Source,
        SourcePath,
        Library,
        LibraryPath,
        _Count
    };
    
    // Arhitecture
    enum class Architecture {
        X86_32,
        X86_64
    };
    static inline std::string getArchitecture(Architecture arch) {
        if (arch == Architecture::X86_32) return "-m32";
        else return "-m64";
    }
    
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
        
        // create new global context
        static Context * create() { return new Context(); }
        
        // Add resource
        Context & add(const FS::path & path, ResourceType type);
        
        // get resources
        const ResourceContainer & get(ResourceType type) const;
        
        // output path
        const FS::path & output() const;
        Context & output(const FS::path & path);
        
        // verbose
        bool verbose() const { return m_verbose; }
        Context & verbose(bool verbose) { m_verbose = verbose; return *this; }
        
        // architecture
        Architecture arch() const { return m_arch; }
        Context & arch(Architecture arch) { m_arch = arch; return *this; }
        
        // optimization level
        OptimizationLevel opt() const { return m_optLevel; }
        Context & opt(OptimizationLevel level) { m_optLevel = level; return *this; }
        
        // emit mode
        EmitType emit() const { return m_emit; }
        Context & emit(EmitType emit) { m_emit = emit; return *this; }
        
        // generate a string containing all current options
        std::string toString() const;
        
    private:
        
        // create
        Context();
        
        /// Resolve directory path
        FS::path resolveDir(const FS::path & path) const;
        
        /// resolve resource path against the container
        FS::path resolveFile(const FS::path & path, ResourceType type) const;
        
        /// array that contains the resources
        ResourceContainer m_resources[(int)ResourceType::_Count];
        
        // output path including filename
        // make it mutable for caching
        mutable FS::path m_output;
        
        // verbose build?
        bool m_verbose = false;
        
        // build for architecture
        Architecture m_arch;
        OptimizationLevel m_optLevel;
        EmitType m_emit;
    };

}; // lbc namespace
