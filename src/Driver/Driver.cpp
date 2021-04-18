//
// Created by Albert on 13/07/2020.
//
#include "Driver.h"
#include "Ast/Ast.h"
#include "Ast/AstPrinter.h"
#include "Ast/CodePrinter.h"
#include "Gen/CodeGen.h"
#include "Parser/Parser.h"
#include "Sem/SemanticAnalyzer.h"
#include "Toolchain/ToolTask.h"
#include "Toolchain/Toolchain.h"
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/Host.h>

using namespace lbc;

Driver::Driver(Context& context)
: m_context{ context } {
}

Driver::~Driver() = default;

int Driver::execute() {
    //    validate();
    //    compileSources();

    //    if (outputLLVMIr()) {
    //        emitLLVMIr();
    //    } else {
    //        emitBitCode(false);
    //    }
    //
    //    switch (m_outputType) {
    //    case OutputType::Native:
    //        switch (m_compilationTarget) {
    //        case CompilationTarget::Executable:
    //            // compile sources
    //            // generate bitcode files from:
    //            // - sources, .ll
    //            // optimize bitcode files
    //            // generate object files from:
    //            // - bitcode, .s
    //            // final link output
    //            // output target
    //            // remove generated temp files
    //            break;
    //        case CompilationTarget::Object:
    //            // compile sources
    //            // generate bitcode files from:
    //            // - sources, .ll
    //            // optimize bitcode files
    //            // generate object files from:
    //            // - bitcode, .s
    //            // output target
    //            // remove generated temp files
    //            break;
    //        case CompilationTarget::Assembly:
    //            // compile sources
    //            // generate bitcode files from:
    //            // - sources, .ll
    //            // optimize bitcode files
    //            // generate asm files from:
    //            // - bitcode
    //            // output target
    //            // remove generated temp files
    //            break;
    //        }
    //        break;
    //    case OutputType::LLVM:
    //        switch (m_compilationTarget) {
    //        case CompilationTarget::Object:
    //            // compile sources
    //            // generate bitcode files from:
    //            // - sources, .ll
    //            // optimize bitcode files
    //            // output target
    //            // remove generated temp files
    //            break;
    //        case CompilationTarget::Assembly:
    //            // compile sources
    //            // generate llvm-ir files from:
    //            // - sources, .bc
    //            // optimize llvm-ir files
    //            // output target
    //            // remove generated temp files
    //            break;
    //        default:
    //            llvm_unreachable("Invalid target");
    //        }
    //        break;
    //    }


    //    switch (m_result) {
    //    case CompileResult::Default:
    //        emitExecutable();
    //        break;
    //    case CompileResult::LLVMIr:
    //        emitLLVMIr();
    //        break;
    //    case CompileResult::BitCode:
    //        emitBitCode(true);
    //        break;
    //    case CompileResult::Assembly:
    //        emitNative(CompileResult::Assembly, true);
    //        break;
    //    case CompileResult::Object:
    //        emitNative(CompileResult::Object, true);
    //        break;
    //    case CompileResult::Executable:
    //        emitExecutable();
    //        break;
    //    case CompileResult::Library:
    //        error("Creating libraries is not supported");
    //    }
    //
    return EXIT_SUCCESS;
}

void Driver::emitLLVMIr() {
    //    if (m_optimizationLevel > OptimizationLevel::O0) {
    //        auto bcFiles = emitBitCode(false);
    //        optimize(bcFiles, CompileResult::LLVMIr, true);
    //        for (const auto& file : bcFiles) {
    //            fs::remove(file);
    //        }
    //    } else {
    //        bool single = m_modules.size() == 1;
    //        for (auto& module : m_modules) {
    //            fs::path path = resolveOutputPath(module->getSourceFileName(), ".ll", single, true);
    //
    //            std::error_code ec{};
    //            llvm::raw_fd_ostream os{ path.string(), ec, llvm::sys::fs::OF_Text };
    //
    //            auto* printer = llvm::createPrintModulePass(os);
    //            printer->runOnModule(*module);
    //
    //            os.flush();
    //            os.close();
    //        }
    //    }
}

std::vector<fs::path> Driver::emitBitCode(bool final) {
    std::vector<fs::path> result;
    //    result.reserve(m_modules.size());
    //
    //    bool single = m_modules.size() == 1;
    //    for (auto& module : m_modules) {
    //        fs::path output = resolveOutputPath(module->getSourceFileName(), ".bc", single, final);
    //
    //        std::error_code errors{};
    //        llvm::raw_fd_ostream stream{ output.string(), errors, llvm::sys::fs::OF_None };
    //        llvm::WriteBitcodeToFile(*module, stream);
    //        stream.flush();
    //        stream.close();
    //
    //        result.emplace_back(output);
    //    }
    //
    return result;
}

std::vector<fs::path> Driver::emitNative(Context::CompilationTarget emit, bool final) {
    //    string fileType;
    //    string ext;
    //    switch (emit) {
    //    case CompileResult::Object:
    //        fileType = "-filetype=obj";
    //        ext = ".o";
    //        break;
    //    case CompileResult::Assembly:
    //        fileType = "-filetype=asm";
    //        ext = ".s";
    //        break;
    //    default:
    //        llvm_unreachable("Emit Native only emits object and asm");
    //    }
    //
    std::vector<fs::path> result{};
    //    result.reserve(m_modules.size());
    //
    //    auto bcFiles = emitBitCode(false);
    //    if (m_optimizationLevel > OptimizationLevel::O0) {
    //        bcFiles = optimize(bcFiles, CompileResult::BitCode, false);
    //    }
    //
    //    auto tool = getToolPath(ToolKind::Assembler).string();
    //    bool single = bcFiles.size() == 1;
    //    for (const auto& path : bcFiles) {
    //        string input{ path.string() };
    //        auto output = resolveOutputPath(path, ext, single, final).string();
    //
    //        std::vector<llvm::StringRef> args{
    //            tool,
    //            fileType,
    //            "-o",
    //            output,
    //            input
    //        };
    //        auto code = llvm::sys::ExecuteAndWait(tool, args);
    //        if (code != EXIT_SUCCESS) {
    //            error("Failed emit '"s + output + "'");
    //        }
    //        result.emplace_back(output);
    //
    //        fs::remove(input);
    //    }
    //
    return result;
}

void Driver::emitExecutable() {
    //    auto objects = emitNative(CompileResult::Object, false);
    //    auto output = resolveOutputPath("a", ".out", true, true).string();
    //    auto tool = getToolPath(ToolKind::Linker).string();
    //
    //    std::vector<string> stringCopies;
    //    stringCopies.reserve(objects.size());
    //
    //    std::vector<llvm::StringRef> args;
    //    constexpr auto reserve = 16;
    //    args.reserve(objects.size() + reserve);
    //    args.emplace_back(tool);
    //
    //    fs::path linuxSysPath;
    //    if (m_triple.isOSLinux()) {
    //        if (m_triple.isArch32Bit()) {
    //            args.emplace_back("-m");
    //            args.emplace_back("elf_i386");
    //            linuxSysPath = "/usr/libr32";
    //            if (!fs::exists(linuxSysPath)) {
    //                linuxSysPath = "/usr/lib/i386-linux-gnu";
    //                if (!fs::exists(linuxSysPath)) {
    //                    error("No 32 bit libraries found");
    //                }
    //            }
    //        } else if (m_triple.isArch64Bit()) {
    //            args.emplace_back("-m");
    //            args.emplace_back("elf_x86_64");
    //            args.emplace_back("-dynamic-linker");
    //            args.emplace_back("/lib64/ld-linux-x86-64.so.2");
    //            linuxSysPath = "/usr/lib/x86_64-linux-gnu";
    //        } else {
    //            error("Unknown architecture");
    //        }
    //        args.emplace_back("-L");
    //        args.emplace_back("/usr/lib");
    //
    //        args.emplace_back(stringCopies.emplace_back((linuxSysPath / "crt1.o").string()));
    //        args.emplace_back(stringCopies.emplace_back((linuxSysPath / "crti.o").string()));
    //    } else if (m_triple.isMacOSX()) {
    //        args.emplace_back("-lSystem");
    //    } else if (m_triple.isOSWindows()) {
    //        error("Building for Windows not currently supported");
    //    }
    //    args.emplace_back("-o");
    //    args.emplace_back(output);
    //
    //    for (const auto& path : objects) {
    //        args.emplace_back(stringCopies.emplace_back(path.string()));
    //    }
    //
    //    if (m_triple.isOSLinux()) {
    //        args.emplace_back("--no-as-needed");
    //        args.emplace_back("-lc");
    //        args.emplace_back(stringCopies.emplace_back((linuxSysPath / "crtn.o").string()));
    //    }
    //
    //    auto code = llvm::sys::ExecuteAndWait(tool, args);
    //    if (code != EXIT_SUCCESS) {
    //        error("Failed generate '"s + output + "'");
    //    }
    //
    //    for (const auto& path : objects) {
    //        fs::remove(path);
    //    }
}

// Compile

void Driver::compileSources() {
    const auto& sources = m_context.getInputFiles(Context::FileType::Source);
    m_modules.reserve(sources.size());
    for (const auto& source : sources) {
        auto path = m_context.resolvePath(source).string();

        string included;
        auto ID = m_context.getSourceMrg().AddIncludeFile(path, {}, included);
        if (ID == ~0U) {
            fatalError("Failed to load '"s + path + "'");
        }

        compileSource(path, ID);
    }
}

void Driver::compileSource(const fs::path& path, unsigned int ID) {
    Parser parser{ m_context, ID };
    auto ast = parser.parse();
    if (!ast) {
        fatalError("Failed to parse '"s + path.string() + "'");
    }

    // Analyze
    SemanticAnalyzer sem{ m_context, ID };
    sem.visitStmt(ast.get());

    // generate IR
    CodeGen gen{ m_context, ID };
    gen.visitStmt(ast.get());

    // done
    if (!gen.validate()) {
        fatalError("Failed to compile '"s + path.string() + "'");
    }

    // Happy Days
    m_modules.emplace_back(gen.getModule());
}

//// Optimize
//
//std::vector<fs::path> Driver::optimize(const std::vector<fs::path>& files, CompileResult emit, bool final) {
//    std::vector<fs::path> result;
//    result.reserve(files.size());
//
//    string emitOpt;
//    string ext;
//    switch (emit) {
//    case CompileResult::LLVMIr:
//        emitOpt = "-S";
//        ext = ".ll";
//        break;
//    case CompileResult::BitCode:
//        emitOpt = "";
//        ext = ".bc";
//        break;
//    case CompileResult::Assembly:
//        emitOpt = "--filetype=asm";
//        ext = ".asm";
//        break;
//    case CompileResult::Object:
//        emitOpt = "--filetype=obj";
//        ext = ".o";
//        break;
//    case CompileResult::Default:
//    case CompileResult::Executable:
//    case CompileResult::Library:
//        llvm_unreachable("Optimizer does not generate executable/library output");
//    }
//
//    auto level = getCmdOption(m_optimizationLevel);
//    auto tool = getToolPath(ToolKind::Optimizer).string();
//    bool single = files.size() == 1;
//    for (const auto& path : files) {
//        string file{ path.string() };
//        auto output = resolveOutputPath(path, ext, single, final).string();
//        std::vector<llvm::StringRef> args{
//            tool,
//            level,
//            "-o",
//            output,
//            file
//        };
//        if (!emitOpt.empty()) {
//            args.insert(std::next(args.begin()), emitOpt);
//        }
//        auto code = llvm::sys::ExecuteAndWait(tool, args);
//        if (code != EXIT_SUCCESS) {
//            error("Failed to optimize '"s + file + "'");
//        }
//
//        result.emplace_back(output);
//    }
//
//    return result;
//}
