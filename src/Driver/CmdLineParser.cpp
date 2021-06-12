//
// Created by Albert Varaksin on 17/04/2021.
//
#include "CmdLineParser.hpp"
#include "CompileOptions.hpp"
#include <llvm/Support/FileSystem.h>
using namespace lbc;

void CmdLineParser::parse(const Args& args) {
    if (args.size() < 2) {
        showError("no input.");
    }

    // compiler executable
    fs::path executable = llvm::sys::fs::getMainExecutable(
        args[0],
        reinterpret_cast<void*>(showHelp)); // NOLINT
    m_options.setCompilerPath(executable);

    // working directory
    m_options.setWorkingDir(fs::current_path());

    // lbc ( option | <file> )+
    size_t index = 1;
    for (; index < args.size(); index++) {
        if (*args[index] == '-') {
            processOption(args, index);
        } else {
            m_options.addInputFile(args[index]);
        }
    }
}

void CmdLineParser::processOption(const Args& args, size_t& index) {
    const StringRef arg{ args[index] };
    if (arg == "-v") {
        m_options.setVerbose(true);
    } else if (arg == "-o") {
        index++;
        if (index >= args.size()) {
            showError("output file path missing.");
        }
        m_options.setOutputPath(args[index]);
    } else if (arg == "-m32") {
        m_options.set32Bit();
    } else if (arg == "-m64") {
        m_options.set64Bit();
    } else if (arg == "--help") {
        showHelp();
    } else if (arg == "--version") {
        showVersion();
    } else if (arg == "-O0") {
        m_options.setOptimizationLevel(CompileOptions::OptimizationLevel::O0);
    } else if (arg == "-OS") {
        m_options.setOptimizationLevel(CompileOptions::OptimizationLevel::OS);
    } else if (arg == "-O1") {
        m_options.setOptimizationLevel(CompileOptions::OptimizationLevel::O1);
    } else if (arg == "-O2") {
        m_options.setOptimizationLevel(CompileOptions::OptimizationLevel::O2);
    } else if (arg == "-O3") {
        m_options.setOptimizationLevel(CompileOptions::OptimizationLevel::O3);
    } else if (arg == "-c") {
        m_options.setCompilationTarget(CompileOptions::CompilationTarget::Object);
    } else if (arg == "-S") {
        m_options.setCompilationTarget(CompileOptions::CompilationTarget::Assembly);
    } else if (arg == "-emit-llvm") {
        m_options.setOutputType(CompileOptions::OutputType::LLVM);
    } else if (arg == "-ast-dump") {
        m_options.setDumpAst(true);
    } else if (arg == "-code-dump") {
        m_options.setDumpCode(true);
    } else if (arg == "-g") {
        m_options.setDebugBuild(true);
    } else if (arg == "--toolchain") {
        index++;
        if (index > args.size()) {
            showError("Toolchain path is missing");
        }
        m_options.setToolchainDir(args[index]);
    } else if (arg == "-main") {
        index++;
        if (index >= args.size()) {
            showError("file path missing.");
        }
        m_options.setMainFile(args[index]);
    } else if (arg == "-no-main") {
        m_options.setImplicitMain(false);
    } else {
        showError("Unrecognized option "s + string(arg) + ".");
    }
}

//void CmdLineParser::processToolchainPath(const fs::path& path) {
//    if (path.is_absolute()) {
//        if (fs::exists(path)) {
//            m_context.getToolchain().setBasePath(path);
//            return;
//        }
//        showError("Toolchain path not found");
//    }
//
//    if (auto rel = fs::absolute(m_context.getCompilerDir() / path); fs::exists(rel)) {
//        m_context.getToolchain().setBasePath(rel);
//        return;
//    }
//
//    if (auto rel = fs::absolute(m_context.getWorkingDir() / path); fs::exists(rel)) {
//        m_context.getToolchain().setBasePath(rel);
//        return;
//    }
//
//    fatalError("Toolchain path not found");
//}


void CmdLineParser::showHelp() {
    // TODO in new *near* future
    // -I <dir>         Add directory to include search path
    // -L <dir>         Add directory to library search path
    // -g               Generate source-level debug information
    llvm::outs() << R"HELP(LightBASIC compiler

USAGE: lbc [options] <inputs>

OPTIONS:
    --help           Display available options
    --version        Show version information
    -v               Show verbose output
    -c               Only run compile and assemble steps
    -S               Only drive compilation steps
    -emit-llvm       Use the LLVM representation for assembler and object files
    -ast-dump        Dump AST tree of the parsed source as json
    -code-dump       Dump AST as source code
    -o <file>        Write output to <file>
    -O<number>       Set optimization. Valid options: O0, OS, O1, O2, O3
    -m32             Generate 32bit i386 code
    -m64             Generate 64bit x86-64 code
    -toolchain <Dir> Path to LLVM toolchain
    -main <file>     File which will have implicit `main` function
    -no-main         Do not generate implicit `main` function
)HELP";
    std::exit(EXIT_SUCCESS);
}

void CmdLineParser::showVersion() {
    llvm::outs() << "LightBASIC version " << LBC_VERSION_STRING
                 << " (Based on LLVM " << LLVM_VERSION_STRING << ")\n"
                 << "(c) Albert Varaksin 2021"
                 << '\n';
    std::exit(EXIT_SUCCESS);
}

void CmdLineParser::showError(const string& message) {
    llvm::errs() << message
                 << " Use --help for more info"
                 << '\n';
    std::exit(EXIT_FAILURE);
}
