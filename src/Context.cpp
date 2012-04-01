//
//  Context.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "Context.h"
using namespace lbc;

#define LBC_DEF_ARCH        (sizeof(void *) == 4 ? Architecture::X86_32 : Architecture::X86_64)
#define LBC_DEF_OPT_LVL     (OptimizationLevel::O3)
#define LBC_DEF_EMIT_TYP    (EmitType::Executable)


/**
 * Get global context object
 */
Context & Context::getGlobalContext()
{
    static Context _globalContext;
    return _globalContext;
}


/**
 * Initialize the context
 */
Context::Context()
:   m_arch(LBC_DEF_ARCH),
    m_optLevel(LBC_DEF_OPT_LVL),
    m_emit(LBC_DEF_EMIT_TYP)
{
}


/**
 * generate a string from current compiler options
 */
string Context::toString() const
{
    std::stringstream s;
    
    // verbose?
    if (verbose()) s << " -v";
    
    // arch
    s << " " << getArchitecture(arch());
    
    // optimization level
    s << " " << getOptimizationLevel(opt());
    
    // emit type
    bool showOutput = emit() == EmitType::Executable || emit() == EmitType::Library;
    string e = getEmitType(emit());
    if (e.length()) s << " " << e;
    
    // output
    if (showOutput) {
        const FS::path &    out = output();
        if (out.is_complete()) s << " -o " << out;
    }
    
    // libraries
    auto & libs = get(ResourceType::Library);
    for (auto & lib : libs) {
        s << "-l" << " " << lib;
    }
    
    // sources
    auto & srcs = get(ResourceType::Source);
    for (auto & src : srcs) {
        s << " " << src;
    }    
    
    // done;
    return s.str();
}


/**
 * get output path including filename
 */
const FS::path & Context::output() const
{
    if (m_output.empty() && m_resources[(int)ResourceType::Source].size()) {
        m_output = m_resources[(int)ResourceType::Source][0];
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
Context & Context::add(const FS::path & path, ResourceType type)
{
    FS::path source;
    switch (type) {
        case ResourceType::GlobalPath:
        case ResourceType::LibraryPath:
        case ResourceType::SourcePath:
            source = resolveDir(path);
            break;
        case ResourceType::Source:
            source = resolveFile(path, ResourceType::SourcePath);
            break;
        case ResourceType::Library:
        {   
            bool dynlib  = path.extension() == ".dylib";
            bool stlib   = dynlib || path.extension() == ".a";
            bool libpref = path.filename().string().substr(0, 3) == "lib";
            FS::path search = path;
            while (true) {
                try {
                    resolveFile(search, ResourceType::LibraryPath);
                    source = path;
                    break;
                } catch (Exception e) {
                    if (!dynlib) {
                        search.replace_extension(".dylib");
                        dynlib = true;
                        continue;
                    } else if (!libpref) {
                        search = path.parent_path() / (string("lib") + search.filename().string());
                        libpref = true;
                        continue;
                    } else if (!stlib) {
                        search.replace_extension(".a");
                        stlib = true;
                    } else {
                        throw e;
                    }
                }
            }
            break;
        }
        default:
            THROW_EXCEPTION("Invalid ResourceType");
            break;
    }
    
    // add to the container
    ResourceContainer & rct = m_resources[(int)type];
    if (find(rct.begin(), rct.end(), source) == rct.end()) {
        rct.push_back(source);
    }

    return *this;
}


/**
 * Get available resources
 */
const ResourceContainer & Context::get(ResourceType type) const
{
    return m_resources[(int)type];
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
        for (auto dir : m_resources[(int)ResourceType::GlobalPath]) {
            FS::path tmp = dir / path;
            if (FS::is_directory(tmp)) {
                return tmp.normalize();
            }
        }
    } else if (FS::is_directory(path)) {
        return path.normalize();
    }
    
    // not found
    THROW_EXCEPTION("Directory '" + dir.string() + "' not found" );
}


/**
 * Resolve file path
 */
FS::path Context::resolveFile(const FS::path & file, ResourceType type) const
{
    // is absolute path?
    if (!file.is_absolute()) {
        // search the container
        for (auto dir : m_resources[(int)type]) {
            FS::path tmp = dir / file;
            if (FS::is_regular_file(tmp)) {
                return tmp.normalize();
            }
        }
        // search global container
        for (auto dir : m_resources[(int)ResourceType::GlobalPath]) {
            FS::path tmp = dir / file;
            if (FS::is_regular_file(tmp)) {
                return tmp.normalize();
            }
        }
    } else if (FS::is_regular_file(file)) {
        return FS::path(file).normalize();
    }
    
    // not found
    THROW_EXCEPTION("File '" + file.string() + "' not found");
}
