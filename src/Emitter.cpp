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
Emitter::Emitter(const shared_ptr<Context> & ctx) : m_context(ctx)
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
    // nothing?
    if (!m_modules.size()) return;
    
    // the output
    auto output = m_context->output();
    string llc_cmd  = "/usr/local/bin/llc -filetype=obj ";
    string llvm_lld = "/usr/local/bin/llvm-ld ";
    string ld_cmd   = "/usr/bin/ld ";
    string as_cmd   = "/usr/bin/as ";
    string cmd      = "";
    bool verbose    = true;
    vector<string> bc_files;
    
    // ld options. need to be configurable and different on different platforms
    ld_cmd += "-lSystem -lcrt1.10.6.o -arch x86_64 -L\"/usr/lib/\" -macosx_version_min 10.6.0 ";
    
    // generate llvm bitcode files
    for (auto & module : m_modules) {
        // is it safe to use module identifier as filename?
        FS::path path(module->getModuleIdentifier());
        path.replace_extension(".bc");
        // verbose?
        if (verbose) std::cout << "Generate " << path << '\n';
        // write bitcode files.
        string errors;
        llvm::raw_fd_ostream stream(path.c_str(), errors, llvm::raw_fd_ostream::F_Binary);
        if (errors.length()) {
            throw Exception(errors);
        }
        llvm::WriteBitcodeToFile(module.get(), stream);
        // push the file to the vector
        bc_files.push_back(path.string());
    }
    
    // link bytecode files with llvm-ld
    FS::path bc_path = output;
    bc_path.replace_extension(".bc");
    cmd = llvm_lld + '"' + boost::algorithm::join(bc_files, "\" \"") + "\" -o \"" + output.string() + '"';
    if (verbose) std::cout << cmd << '\n';
    ::system(cmd.c_str());
    
    // to s
    FS::path obj_path = output;
    obj_path.replace_extension(".o");
    cmd = llc_cmd + '"' + bc_path.string() + "\" -o \"" + obj_path.string() + '"';
    if (verbose) std::cout << cmd << '\n';
    ::system(cmd.c_str());
    
    // link
    cmd = ld_cmd + '"' + obj_path.string() + "\" -o \"" + output.string() + '"';
    if (verbose) std::cout << cmd << '\n';
    ::system(cmd.c_str());
}

