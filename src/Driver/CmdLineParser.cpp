//
// Created by Albert Varaksin on 17/04/2021.
//
#include "CmdLineParser.h"
#include "Context.h"
#include <llvm/Support/FileSystem.h>
using namespace lbc;

void CmdLineParser::parse(const Args& args) noexcept {
    if (args.size() < 2) {
        showError("no input.");
    }

    // compiler executable
    fs::path executable = llvm::sys::fs::getMainExecutable(
        args[0],
        reinterpret_cast<void*>(showHelp)); // NOLINT
    m_context.setCompilerPath(executable);

    // lbc ( option | <file> )+
    size_t index = 1;
    for (; index < args.size(); index++) {
        if (*args[index] == '-') {
            processOption(args, index);
        } else {
            m_context.addInputFile(args[index]);
        }
    }
}

void CmdLineParser::processOption(const Args& args, size_t& index) noexcept {
    const StringRef arg{ args[index] };
    if (arg == "-v") {
        m_context.setVerbose(true);
    } else if (arg == "-o") {
        index++;
        if (index >= args.size()) {
            showError("output file path missing.");
        }
        m_context.setOutputFilePath(args[index]);
    } else if (arg == "-m32") {
        auto triple = m_context.getTriple();
        m_context.setTriple(triple.get32BitArchVariant());
    } else if (arg == "-m64") {
        auto triple = m_context.getTriple();
        m_context.setTriple(triple.get64BitArchVariant());
    } else if (arg == "--help") {
        showHelp();
    } else if (arg == "--version") {
        showVersion();
    } else if (arg == "-O0") {
        m_context.setOptimizationLevel(Context::OptimizationLevel::O0);
    } else if (arg == "-OS") {
        m_context.setOptimizationLevel(Context::OptimizationLevel::OS);
    } else if (arg == "-O1") {
        m_context.setOptimizationLevel(Context::OptimizationLevel::O1);
    } else if (arg == "-O2") {
        m_context.setOptimizationLevel(Context::OptimizationLevel::O2);
    } else if (arg == "-O3") {
        m_context.setOptimizationLevel(Context::OptimizationLevel::O3);
    } else if (arg == "-c") {
        m_context.setCompilationTarget(Context::CompilationTarget::Object);
    } else if (arg == "-S") {
        m_context.setCompilationTarget(Context::CompilationTarget::Assembly);
    } else if (arg == "-emit-llvm") {
        m_context.setOutputType(Context::OutputType::LLVM);
    } else if (arg == "-g") {
        m_context.setDebugBuild(true);
    } else if (arg == "--toolchain") {
        index++;
        if (index > args.size()) {
            showError("Toolchain path is missing");
        }
        processToolchainPath(args[index]);
    } else if (arg == "-main") {
        index++;
        if (index >= args.size()) {
            showError("file path missing.");
        }
        m_context.setMainFile(args[index]);
    } else if (arg == "-no-main") {
        m_context.setImplicitMain(false);
    } else {
        showError("Unrecognized option "s + string(arg) + ".");
    }
}

void CmdLineParser::processToolchainPath(const fs::path& path) noexcept {
    if (path.is_absolute()) {
        if (fs::exists(path)) {
            m_context.getToolchain().setBasePath(path);
            return;
        }
        showError("Toolchain path not found");
    }

    if (auto rel = fs::absolute(m_context.getCompilerDir() / path); fs::exists(rel)) {
        m_context.getToolchain().setBasePath(rel);
        return;
    }

    if (auto rel = fs::absolute(m_context.getWorkingDir() / path); fs::exists(rel)) {
        m_context.getToolchain().setBasePath(rel);
        return;
    }

    fatalError("Toolchain path not found");
}


void CmdLineParser::showHelp() noexcept {
    // TODO in new *near* future
    // -I <dir>         Add directory to include search path
    // -L <dir>         Add directory to library search path
    // -g               Generate source-level debug information
    std::cout << R"HELP(LightBASIC compiler

USAGE: lbc [options] <inputs>

OPTIONS:
    --help           Display available options
    --version        Show version information
    -v               Show verbose output
    -c               Only run compile and assemble steps
    -S               Only drive compilation steps
    -emit-llvm       Use the LLVM representation for assembler and object files
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

void CmdLineParser::showVersion() noexcept {
    std::cout << "LightBASIC version " << LBC_VERSION_STRING
              << " (Based on LLVM " << LLVM_VERSION_STRING << ")\n"
              << "(c) Albert Varaksin 2021"
              << '\n';
    std::exit(EXIT_SUCCESS);
}

void CmdLineParser::showError(const string& message) noexcept {
    std::cerr << message
              << " Use --help for more info"
              << '\n';
    std::exit(EXIT_FAILURE);
}
