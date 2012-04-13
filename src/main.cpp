//
//  main.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team . All rights reserved.
//

#include <iostream>
#include "Context.h"
#include "Parser.h"
#include "Ast.h"
#include "PrinterVisitor.h"
#include "SemanticAnalyser.h"
#include "IrBuilder.h"
#include "SourceFile.h"
#include "Emitter.h"
#include "Version.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/MemoryBuffer.h>

using namespace lbc;

// show the help
void showHelp();

// show compiler version info
void showVersion();


// the main entry point
int main(int argc, const char * argv[])
{
//    auto p = llvm::sys::Path::GetMainExecutable(argv[0], (void *)main);
//    auto compiler_path = llvm::sys::path::parent_path(p.str());
//    
//    llvm::SourceMgr srcMgr;
//    std::vector<std::string> paths;
//    paths.push_back(compiler_path);
//    srcMgr.setIncludeDirs(paths);
//    
//    std::string inc = "";
//    auto ID = srcMgr.AddIncludeFile("samples/HelloWorld.bas", llvm::SMLoc(), inc);
//    auto buffer = srcMgr.getMemoryBuffer(ID);
//    std::cout << buffer->getBuffer().str() << '\n'; 
//    
//    std::cout << "ID = " << ID << '\n';
//    
//    return 0;
    
    
////    // get compiler path
////    auto p = llvm::sys::Path::GetMainExecutable(argv[0], (void *)main);
////    auto compiler_path = llvm::sys::path::parent_path(p.str());
//    
//    // system paths
//    std::vector<llvm::sys::Path> paths;
//    llvm::sys::Path::GetSystemLibraryPaths(paths);
//    for (auto & l : paths) {
//        std::cout << l.str() << '\n';
//    }
//    
//    std::string lsystem = "System";
//    std::string crt0 = "crt1.10.6.o";
//    std::cout << llvm::sys::Path::FindLibrary(crt0).str() << '\n';
//    
//    
//    
//    return 0;
    
    
    try {
        // create the context
        auto & ctx = Context::getGlobalContext();
        // add current path to the global paths list
        ctx.add(FS::current_path(), ResourceType::GlobalPath);
        // parse the cmd line arguments
        if (argc > 1) {
            for (int i = 1; i < argc; i++) {
                std::string arg(argv[i]);
                if (arg == "-v") {
                    ctx.verbose(true);
                } else if (arg == "-o") {
                    if (i + 1 >= argc) {
                        std::cout << "output name missing\n";
                        return EXIT_FAILURE;
                    }
                    ctx.output(argv[i + 1]);
                    i++;
                } else if (arg == "-m32") {
                    ctx.arch(Architecture::X86_32);
                } else if (arg == "-m64") {
                    ctx.arch(Architecture::X86_64);
                } else if (arg == "--help") {
                    showHelp();
                    return EXIT_SUCCESS;
                } else if (arg == "--version") {
                    showVersion();
                    return EXIT_SUCCESS;
                } else if (arg == "-O3") {
                    ctx.opt(OptimizationLevel::O3);
                } else if (arg == "-O2") {
                    ctx.opt(OptimizationLevel::O2);
                } else if (arg == "-O1") {
                    ctx.opt(OptimizationLevel::O1);
                } else if (arg == "-O0") {
                    ctx.opt(OptimizationLevel::O0);
                } else if (arg == "-S") {
                    ctx.emit(EmitType::Asm);
                } else if (arg == "-c") {
                    ctx.emit(EmitType::Object);
                } else if (arg == "-llvm") {
                    ctx.emit(EmitType::Llvm);
                } else if (arg[0] == '-') {
                    std::cout << "lbc: error: Unrecognized option " << arg << '\n';
                    return EXIT_FAILURE;
                } else {
                    ctx.add(arg, ResourceType::Source);
                }
            }
        } else {
            std::cout << "lbc: error: no input files\n";
            return EXIT_FAILURE;
        }
        
        // emitting executable?
        if (ctx.emit() == EmitType::Executable) {
#if defined __linux__
            // TODO add default libraries and objects to link here
            //      on linux the order in wich crt1.o, crti.o and crtn.o are
            //      added matters. So need some sort of priotiy queue
            //      or something.
#elif defined __APPLE__
            ctx.add("/usr/lib",     ResourceType::LibraryPath);
            ctx.add("System",       ResourceType::Library);
            ctx.add("crt1.10.6.o",  ResourceType::Library);
#else
    #error Unsupported system
#endif
        }
        
        // create the parser
        auto parser = std::make_shared<Parser>(ctx);
        // semantic analyser
        auto semantic = std::make_shared<SemanticAnalyser>(ctx);
        // IR builder
        auto builder = std::make_shared<IrBuilder>(ctx);
        // create output emitter
        auto emitter = std::make_shared<Emitter>(ctx);
        // process the files
        for (auto & file : ctx.get(ResourceType::Source)) {
            auto ast = parser->parse(std::make_shared<SourceFile>(file));
            if (ast) {
                // analyse
                ast->accept(semantic.get());
                
                // generate llvm code
                ast->accept(builder.get());
                if (auto module = builder->getModule()) {
                    emitter->add(module);
                }
                
                // cleanup
                delete ast;
            } else {
                std::cout << "NO AST\n";
            }
        }
        // generate the output
        emitter->generate();
    } catch (Exception e) {
        std::cout << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


/**
 * Print command line help
 */
void showHelp()
{
    std::cout <<
        "OVERVIEW: LightBASIC compiler\n"
        "\n"
        "USAGE lbc [options] <inputs>\n"
        "\n"
        "OPTIONS:\n"
        "  --help\t"        "Display available options\n"
        "  --version\t"     "Show version information\n"
        "  -v\t\t"          "Show verbose output\n"
        "  -o <file>\t"     "Write output to <file>\n"
        "  -O<number>\t"    "Set optimization level to <number>\n"
        "  -m32\t\t"        "Generate 32bit i386 code\n"
        "  -m64\t\t"        "Generate 64bit x86-64 code\n"
        "  -S\t\t"          "Only compile. Emit assembly files\n"
        "  -c\t\t"          "Compile and assemble. Emit object files\n"
        "  -llvm\t\t"       "Emit llvm files\n"
    ;
}


/**
 * Show compiler version information
 */
void showVersion()
{
    std::cout <<
        "LightBASIC version " << LBC_VERSION_STRING << " (Based on LLVM " << PACKAGE_VERSION << ")\n"
        "(c) Albert Varaksin 2012\n"
    ;
}











