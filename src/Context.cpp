//
//  Context.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "Context.h"
using namespace lbc;



/**
 * get output path including filename
 */
const FS::path & Context::output()
{
    if (m_output.empty() && m_resources[ResourceTypes::Source].size()) {
        m_output = m_resources[ResourceTypes::Source][0];
        m_output.replace_extension();
    }
    return m_output;
}


/**
 * set output path
 */
Context & Context::output(const FS::path & path)
{
    m_output = path;
    return *this;
}


/**
 * Add path to the resource container
 */
Context & Context::add(const FS::path & path, ResourceTypes type)
{
    FS::path source;
    switch (type) {
        case GlobalPath:
        case LibraryPath:
        case SourcePath:
            source = resolveDir(path);
            break;
        case Source:
            source = resolveFile(path, SourcePath);
            break;
        case Library:
            source = resolveFile(path, LibraryPath);
            break;
        default:
            throw Exception("Invalid ResourceType");
            break;
    }
    
    // add to the container
    ResourceContainer & rct = m_resources[type];
    if (find(rct.begin(), rct.end(), source) == rct.end()) {
        rct.push_back(source);
    }

    return *this;
}


/**
 * Get available resources
 */
const ResourceContainer & Context::get(ResourceTypes type) const
{
    if (type < 0 || type >= _ResourceCount) {
        throw Exception("Invalid ResourceType");
    }
    return m_resources[type];
}


/**
 * Resolve directory
 */
FS::path Context::resolveDir(const FS::path & dir) const
{
    // the path
    FS::path path(dir);
    
    // if nor absolute then check against registered global directories
    if (!path.is_absolute()) {
        for (auto dir : m_resources[GlobalPath]) {
            FS::path tmp = dir / path;
            if (FS::is_directory(tmp)) {
                return tmp.normalize();
            }
        }
    } else if (FS::is_directory(path)) {
        return path.normalize();
    }
    
    // not found
    throw Exception("Directory '" + dir.string() + "' not found" );
}


/**
 * Resolve file path
 */
FS::path Context::resolveFile(const FS::path & file, ResourceTypes type) const
{
    // is absolute path?
    if (!file.is_absolute()) {
        // search the container
        for (auto dir : m_resources[type]) {
            FS::path tmp = dir / file;
            if (FS::is_regular_file(tmp)) {
                return tmp.normalize();
            }
        }
        // search global container
        for (auto dir : m_resources[GlobalPath]) {
            FS::path tmp = dir / file;
            if (FS::is_regular_file(tmp)) {
                return tmp.normalize();
            }
        }
    } else if (FS::is_regular_file(file)) {
        return FS::path(file).normalize();
    }
    
    // not found
    throw Exception("File '" + file.string() + "' not found");
}
