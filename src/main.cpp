//
// Created by Albert on 03/07/2020.
//
#include "pch.h"
#include "Driver/Driver.h"
#include <llvm/Support/FileSystem.h>

using namespace lbc;

[[noreturn]] static void showHelp();
[[noreturn]] static void showVersion();
[[noreturn]] static void showError(const string& message);

using Args = llvm::ArrayRef<const char*>;
static void processCmdLine(Driver& driver, const Args& args);
static void processOption(Driver& driver, const Args& args, size_t& index);

int main(int argc, const char* argv[]) {
    Driver driver;
    processCmdLine(driver, { argv, static_cast<size_t>(argc) });
    return driver.execute();
}

void processCmdLine(Driver& driver, const Args& args) {
    if (args.size() < 2) {
        showError("no input");
    }

    // compiler executable
    fs::path executable = llvm::sys::fs::getMainExecutable(args[0], reinterpret_cast<void*>(processCmdLine)); // NOLINT
    driver.setCompilerPath(executable);

    // current working directory
    auto workingDir = fs::current_path();
    driver.setWorkingDir(workingDir);

    // lbc ( option | <file> )+
    size_t index = 1;
    for (; index < args.size(); index++) {
        if (*args[index] == '-') {
            processOption(driver, args, index);
        } else {
            driver.addInputFile(args[index]);
        }
    }
}

static void processOption(Driver& driver, const Args& args, size_t& index) {
    const string_view arg{ args[index] };
    if (arg == "-v") {
        driver.setVerbose(true);
    } else if (arg == "-o") {
        auto next = index + 1;
        if (next >= args.size()) {
            showError("output file path missing.");
        }
        driver.setOutputFilePath(args[next]);
        index = next;
    } else if (arg == "-m32") {
        auto& triple = driver.getTriple();
        triple = triple.get32BitArchVariant();
    } else if (arg == "-m64") {
        auto& triple = driver.getTriple();
        triple = triple.get64BitArchVariant();
    } else if (arg == "--help") {
        showHelp();
    } else if (arg == "--version") {
        showVersion();
    } else if (arg == "-O0") {
        driver.setOptimizationLevel(Driver::OptimizationLevel::O0);
    } else if (arg == "-OS") {
        driver.setOptimizationLevel(Driver::OptimizationLevel::OS);
    } else if (arg == "-O1") {
        driver.setOptimizationLevel(Driver::OptimizationLevel::O1);
    } else if (arg == "-O2") {
        driver.setOptimizationLevel(Driver::OptimizationLevel::O2);
    } else if (arg == "-O3") {
        driver.setOptimizationLevel(Driver::OptimizationLevel::O3);
    } else if (arg == "-c") {
        driver.setCompilationTarget(Driver::CompilationTarget::Object);
    } else if (arg == "-S") {
        driver.setCompilationTarget(Driver::CompilationTarget::Assembly);
    } else if (arg == "-emit-llvm") {
        driver.setOutputType(Driver::OutputType::LLVM);
    } else {
        showError("Unrecognized option "s + string(arg) + ".");
    }
}

[[noreturn]]
void showHelp() {
    // TODO in new *near* future
    // -toolchain <dir> Use the llvm toolchain at the given directory
    // -I <dir>         Add directory to include search path
    // -L <dir>         Add directory to library search path
    // -g               Generate source-level debug information
    std::cout << R"HELP(LightBASIC compiler

USAGE: lbc [options] <inputs>

OPTIONS:
    --help     Display available options
    --version  Show version information
    -v         Show verbose output
    -c         Only run compile and assemble steps
    -S         Only run compilation steps
    -emit-llvm Use the LLVM representation for assembler and object files
    -o <file>  Write output to <file>
    -O<number> Set optimization. Valid options: O0, OS, O1, O2, O3
    -m32       Generate 32bit i386 code
    -m64       Generate 64bit x86-64 code
)HELP";
    std::exit(EXIT_SUCCESS);
}

[[noreturn]]
void showVersion() {
    std::cout << "LightBASIC version " << LBC_VERSION_STRING
              << " (Based on LLVM " << LLVM_VERSION_STRING << ")\n"
              << "(c) Albert Varaksin 2020"
              << '\n';
    std::exit(EXIT_SUCCESS);
}

[[noreturn]]
void showError(const string& message) {
    std::cerr << message
              << " Use --help for more info"
              << '\n';
    std::exit(EXIT_FAILURE);
}
