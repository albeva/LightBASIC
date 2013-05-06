//
//  main.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 25/02/2012.
//  Copyright (c) 2012 LightBASIC development team . All rights reserved.
//

#include "Context.h"
#include "Driver.h"
#include "Version.h"
#include "Path.h"

#include <llvm/Support/Path.h>
#include <iostream>

using namespace lbc;

// show the help
void showHelp();

// show compiler version info
void showVersion();

// get compiler path
std::string getCompilerPath(const char * argv0);

// get current path
std::string getCurrentPath();


// the main entry point
int main(int argc, const char * argv[])
{
    try {
        // create the context
        auto & ctx = Context::getGlobalContext();
        
        // add current path to the global paths list
        ctx.add(getCurrentPath(), ResourceType::GlobalPath);
        ctx.compiler(getCompilerPath(argv[0]));
        
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
                    ctx.output(Path(argv[i + 1]));
                    i++;
                } else if (arg == "-m32") {
                    ctx.triple() = ctx.triple().get32BitArchVariant();
                } else if (arg == "-m64") {
                    ctx.triple() = ctx.triple().get64BitArchVariant();
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
        
        // let the driver to deal with it now
        Driver driver(ctx);
        driver.drive();

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
    std::cout << R"CMD(OVERVIEW: LightBASIC compiler
    
USAGE lbc [options] <inputs>
    
OPTIONS:
    --help      Display available options
    --version   Show version information
    -v          Show verbose output
    -o <file>   Write output to <file>
    -O<number>  Set optimization level to <number>
    -m32        Generate 32bit i386 code
    -m64        Generate 64bit x86-64 code
    -S          Only compile. Emit assembly files
    -c          Compile and assemble. Emit object files
    -llvm       Emit llvm files
)CMD";
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


/**
 * get current working directory
 */
std::string getCurrentPath()
{
    return llvm::sys::Path::GetCurrentDirectory().str();
}


/**
 * Get the compiler path
 */
std::string getCompilerPath(const char * argv0)
{
    auto path = llvm::sys::Path::GetMainExecutable(argv0, (void*)(intptr_t)getCompilerPath);
    return llvm::sys::path::parent_path(path.str()).str();
}
