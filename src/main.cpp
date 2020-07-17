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

static void processCmdLine(Driver& driver, const llvm::ArrayRef<const char*>& args);
static void processOptions(Driver& driver, const llvm::ArrayRef<const char*>& args, size_t& index);

int main(int argc, const char* argv[]) {
    Driver driver;
    processCmdLine(driver, { argv, static_cast<size_t>(argc) });
    return driver.execute();
}

static void processCmdLine(Driver& driver, const llvm::ArrayRef<const char*>& args) {
    if (args.size() < 2) {
        showError("lbc: no input.");
    }

    // compiler executable
    fs::path executable = llvm::sys::fs::getMainExecutable(args[0], reinterpret_cast<void*>(processCmdLine)); // NOLINT
    driver.setCompilerPath(executable);

    // current working directory
    auto workingDir = fs::current_path();
    driver.setWorkingDir(workingDir);

    // lbc [options] <input>+
    size_t index = 1;
    for (; index < args.size(); index++) {
        if (*args[index] == '-') {
            processOptions(driver, args, index);
        } else {
            driver.addResource(Driver::ResourceType::Source, args[index]);
        }
    }
}

static void processOptions(Driver& driver, const llvm::ArrayRef<const char*>& args, size_t& index) {
    const string_view arg{ args[index] };
    if (arg == "-v") {
        driver.setVerbose(true);
    } else if (arg == "-o") {
        auto next = index + 1;
        if (next >= args.size()) {
            showError("output path missing.");
        }
        driver.setOutputPath(args[next]);
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
        driver.setLevel(Driver::OptimizationLevel::O0);
    } else if (arg == "-O1") {
        driver.setLevel(Driver::OptimizationLevel::O1);
    } else if (arg == "-O2") {
        driver.setLevel(Driver::OptimizationLevel::O2);
    } else if (arg == "-O3") {
        driver.setLevel(Driver::OptimizationLevel::O3);
    } else if (arg == "-emit-asm") {
        driver.setResult(Driver::CompileResult::Assembly);
    } else if (arg == "-emit-obj") {
        driver.setResult(Driver::CompileResult::Object);
    } else if (arg == "-emit-llvm") {
        driver.setResult(Driver::CompileResult::LLVMIr);
    } else if (arg == "-emit-bc") {
        driver.setResult(Driver::CompileResult::BitCode);
    } else {
        showError("Unrecognized option "s + string(arg) + ".");
    }
}


void showHelp() {
    std::cout << R"HELP(LightBASIC compiler

USAGE: lbc [options] <inputs>

OPTIONS:
    --help           Display available options
    --version        Show version information
    -v               Show verbose output
    -c               Only run compile and assemble steps
    -S               Only run compilation steps
    -emit-llvm       Use the LLVM representation for assembler and object files
    -toolchain <dir> Use the llvm toolchain at the given directory
    -I <dir>         Add directory to include search path
    -L <dir>         Add directory to library search path
    -o <file>        Write output to <file>
    -g               Generate source-level debug information
    -O<number>       Set optimization. Valid options: O0, O1, O2, O3
    -m32             Generate 32bit i386 code
    -m64             Generate 64bit x86-64 code
)HELP";
    std::exit(EXIT_SUCCESS);
}

void showVersion() {
    std::cout << "LightBASIC version " << LBC_VERSION_STRING
              << " (Based on LLVM " << LLVM_VERSION_STRING << ")\n"
              << "(c) Albert Varaksin 2020"
              << std::endl;
    std::exit(EXIT_SUCCESS);
}

void showError(const string& message) {
    std::cerr << message
              << " Use --help for more info"
              << std::endl;
    std::exit(EXIT_FAILURE);
}
