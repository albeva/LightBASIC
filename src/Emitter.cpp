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
Emitter::Emitter(Context & ctx) : m_ctx(ctx)
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
    m_modules.push_back(unique_ptr<llvm::Module>(module));
}


/**
 * generate the results
 */
void Emitter::generate()
{
    switch (m_ctx.emit()) {
        case EmitType::Executable:
            emitExecutable();
            break;
        case EmitType::Library:
            emitLibrary();
            break;
        case EmitType::Object:
        case EmitType::Asm:
            emitObjOrAsm();
            break;
        case EmitType::Llvm:
            emitLlvm();
            break;
    }
}


/**
 * emit executable binary
 */
void Emitter::emitExecutable()
{
    // generate .obj files
    emitObjOrAsm();
    stringstream link_cmd, rm_cmd;
    
    // rm
    rm_cmd << "rm";
    
    // linker flags
    link_cmd << getTool(Tool::Ld);
    
    // architecture
    if (m_ctx.arch() == Architecture::X86_32) {
        link_cmd << " -arch i386";
    } else if (m_ctx.arch() == Architecture::X86_64) {
        link_cmd << " -arch x86_64";
    } else {
        THROW_EXCEPTION("Unsuppoered architecture");
    }
    
    // library include paths
    for (auto & path : m_ctx.get(ResourceType::LibraryPath)) {
        link_cmd << " -L" << path;
    }
    
    // libraries
    for (auto & lib : m_ctx.get(ResourceType::Library)) {
        link_cmd << " -l" << lib;
    }
    
    // for macos
    link_cmd << " -macosx_version_min 10.6.0";
    
    // the output
    link_cmd << " -o " << m_ctx.output();
    
    // add object files
    for (auto & module : m_modules) {
        // get path
        FS::path path(module->getModuleIdentifier());
        path.replace_extension(".o");
        link_cmd << " " << path;
        rm_cmd << " " << path;
    }
    
    // do the thing
    ::system(link_cmd.str().c_str());
    ::system(rm_cmd.str().c_str());
    
    // show info
    if (m_ctx.verbose()) {
        std::cout << link_cmd.str() << '\n';
    }
}


/**
 * Emit library (static or shared (dll on windows))
 */
void Emitter::emitLibrary()
{
    THROW_EXCEPTION("Emitting library is not implemented");
}


/**
 * emit asm files
 */
void Emitter::emitObjOrAsm()
{
    // show verbose?
    bool verbose = m_ctx.verbose();
    
    // the command
    stringstream llc_cmd;
    llc_cmd << getTool(Tool::LlvmLlc);
    
    // output type
    if (m_ctx.emit() == EmitType::Asm) {
        llc_cmd << " -filetype=asm";
    } else {
        llc_cmd << " -filetype=obj";
    }
    
    // optimization
    OptimizationLevel lvl = m_ctx.opt();
    llc_cmd << " " << getOptimizationLevel(lvl);
    
    // architecture
    if (m_ctx.arch() == Architecture::X86_32) {
        llc_cmd << " -march=x86";
    } else if (m_ctx.arch() == Architecture::X86_64) {
        llc_cmd << " -march=x86-64";
    } else {
        THROW_EXCEPTION("Unsuppoered architecture");
    }
    
    for (auto & module : m_modules) {
        // get path
        FS::path path(module->getModuleIdentifier());
        path.replace_extension(".bc");
        
        // write .bc files
        {
            string errors;
            llvm::raw_fd_ostream stream(path.c_str(), errors);
            if (errors.length()) {
                THROW_EXCEPTION(errors);
            }
            llvm::WriteBitcodeToFile(module.get(), stream);
        }
        
        // assemble
        {
            stringstream s;
            auto asmPath = path;
            if (m_ctx.emit() == EmitType::Asm) {
                asmPath.replace_extension(".s");
            } else {
                asmPath.replace_extension(".o");
            }
            s << llc_cmd.str() << " -o " << asmPath << " " << path;
            ::system(s.str().c_str());
            
            // info
            if (verbose) std::cout << s.str() << '\n';
        }
        
        // delete tmp file
        {
            stringstream s;
            s << "rm " << path;
            ::system(s.str().c_str());
        }
    }
}


/**
 * emit llvm ir files
 */
void Emitter::emitLlvm()
{
    bool verbose = m_ctx.verbose();
    OptimizationLevel lvl = m_ctx.opt();
    string optimize = "";
    if (lvl != OptimizationLevel::O0) {
        optimize = getTool(Tool::LlvmOpt) + " -S " + getOptimizationLevel(lvl) + " ";
    }
    
    for (auto & module : m_modules) {
        // get path
        FS::path path(module->getModuleIdentifier());
        path.replace_extension(".ll");
        
        // write .ll file
        {
            string errors;
            llvm::raw_fd_ostream stream(path.c_str(), errors);
            if (errors.length()) {
                THROW_EXCEPTION(errors);
            }
            module->print(stream, nullptr);
        }
        
        // optimize
        if (optimize.length()) {
            stringstream s;
            s << optimize << path << " -o " << path;
            ::system(s.str().c_str());
        }
        
        // info
        if (verbose) std::cout << "Generated: " << path << '\n';
    }
}


/**
 * get tool path
 */
string Emitter::getTool(Tool tool)
{
    switch (tool) {
        case Tool::LlvmOpt:
            return "/usr/local/bin/opt";
        case Tool::LlvmLlc:
            return "/usr/local/bin/llc";
        case Tool::Ld:
            return "/usr/bin/ld";
    }
    return "";
}
