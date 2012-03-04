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
		
		private:
        /// Resolve directory path
        FS::path resolveDir(const FS::path & path) const;
        
        /// resolve resource path against the container
        FS::path resolveFile(const FS::path & path, ResourceTypes type) const;
        
        /// array that contains the resources
        ResourceContainer m_resources[_ResourceCount];
	};

}; // lbc namespace
