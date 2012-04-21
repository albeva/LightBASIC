//
//  Context.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "Context.h"
#include <llvm/Support/Host.h>
#include <llvm/Support/Path.h>
#include <iostream>

using namespace lbc;

#define LBC_DEF_OPT_LVL     (OptimizationLevel::O3)
#define LBC_DEF_EMIT_TYP    (EmitType::Executable)

// global context
static Context * _globalContext = nullptr;

/**
 * Get global context object
 */
Context & Context::getGlobalContext()
{
    if (_globalContext == nullptr) {
        _globalContext = new Context();
    }
    return *_globalContext;
}


/**
 * Initialzie the context. Return false if unsuccessful
 * - initialize SourceMgr, check command line options, ...
 */
bool Context::initialize()
{
    // probably not needed because paths are absolute already anyway   
    // m_sourceMgr.setIncludeDirs(get(ResourceType::SourcePath));
    return true;
}


/**
 * Initialize the context
 */
Context::Context()
:   m_verbose(false),
    m_triple(llvm::sys::getDefaultTargetTriple()),
    m_optLevel(LBC_DEF_OPT_LVL),
    m_emit(LBC_DEF_EMIT_TYP)
{
}


/**
 * generate a string from current compiler options
 */
std::string Context::toString() const
{
    std::stringstream s;
    
    // verbose?
    if (verbose()) s << " -v";
    
    // arch
    s << "-arch=" << m_triple.getArchName().str();
    
    // optimization level
    s << " " << getOptimizationLevel(opt());
    
    // emit type
    bool showOutput = emit() == EmitType::Executable || emit() == EmitType::Library;
    std::string e = getEmitType(emit());
    if (e.length()) s << " " << e;
    
    // output
    if (showOutput) {
        s << " -o " << output();
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
const Path & Context::output() const
{
    if (!m_output.isValid() && m_resources[(int)ResourceType::Source].size()) {
        m_output = m_resources[(int)ResourceType::Source][0];
        m_output.extension(llvm::sys::Path::GetEXESuffix());
    }
    return m_output;
}


/**
 * set output path
 */
Context & Context::output(const Path & path)
{
    m_output = path;
    return *this;
}


/**
 * Add path to the resource container
 */
Context & Context::add(const Path & path, ResourceType type)
{
    Path source;
    switch (type) {
        case ResourceType::GlobalPath:
            add(path, ResourceType::SourcePath);
            add(path, ResourceType::LibraryPath);
            return *this;
            break;
        case ResourceType::SourcePath:
            source = resolveDir(path);
            break;
        case ResourceType::LibraryPath:
            source = resolveDir(path);
            break;
        case ResourceType::Source:
            source = resolveFile(path, ResourceType::SourcePath);
            break;
        case ResourceType::Library:
            source = path;
            // do nothing. ld will catch errors
            break;
        default:
        {
            THROW_EXCEPTION("Invalid ResourceType");
            break;
        }
    }
    
    // add to the container
    ResourceContainer & rct = m_resources[(int)type];
    if (find(rct.begin(), rct.end(), source) == rct.end()) {
        rct.push_back(source.str());
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
Path Context::resolveDir(const Path & dir) const
{
    // if nor absolute then check against registered global directories
    if (dir.isRelative()) {
        for (auto & d : m_resources[(int)ResourceType::GlobalPath]) {
            Path tmp(d);
            tmp.append(dir);
            if (tmp.isDirectory() && tmp.exists()) {
                return tmp;
            }
        }
    } else {
        if (dir.exists() && dir.isDirectory()) {
            return dir;
        }
    }
    return Path();
}


/**
 * Resolve file path
 */
Path Context::resolveFile(const Path & file, ResourceType type) const
{
    // is absolute path
    if (file.isRelative()) {
        // search the container
        for (auto & dir : m_resources[(int)type]) {
            Path tmp(dir);
            tmp.append(file);
            if (tmp.exists() && tmp.isFile()) {
                return tmp;
            }
        }
    } else {
        if (file.exists() && file.isFile()) {
            return file;
        }
    }
    
    // not found
    return Path();
}
