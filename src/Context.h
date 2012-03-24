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
    typedef vector<FS::path> ResourceContainer;
    

    /**
     * Porpuse of this class is to gather context information
     * about the compilation. Include folders, files to compile,
     * compile options, etc.
     */
    struct Context : NonCopyable
    {
        // Types of resources
        enum ResourceTypes {
            GlobalPath,
            Source,
            SourcePath,
            Library,
            LibraryPath,
            _ResourceCount
        };
        
        // Add resource
        Context & add(const FS::path & path, ResourceTypes type);
        
        // get resources
        const ResourceContainer & get(ResourceTypes type) const;
        
        // get output path
        const FS::path & output();
        
        // set output path
        Context & output(const FS::path & path);
        
        // get verbose
        bool verbose() const { return m_verbose; }
        
        // set verbose
        void verbose(bool verbose) { m_verbose = verbose; }
        
    private:
        /// Resolve directory path
        FS::path resolveDir(const FS::path & path) const;
        
        /// resolve resource path against the container
        FS::path resolveFile(const FS::path & path, ResourceTypes type) const;
        
        /// array that contains the resources
        ResourceContainer m_resources[_ResourceCount];
        
        // output path including filename
        FS::path m_output;
        
        // verbose build?
        bool m_verbose = false;
    };

}; // lbc namespace
