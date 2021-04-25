//
// Created by Albert Varaksin on 13/07/2020.
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
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>

using namespace lbc;

Driver::Driver(Context& context)
: m_context{ context } {
}

int Driver::execute() {
    processInputs();
    compileSources();
    emitBitCode();

    switch (m_context.getCompilationTarget()) {
    case Context::CompilationTarget::Executable:
        emitObjects();
        emitExecutable();
        break;
    case Context::CompilationTarget::Object:
        // generate bitcode files from:
        // - sources, .ll
        // optimize bitcode files
        // generate object files from:
        // - bitcode, .s
        // output target
        // remove generated temp files
        fatalError("Compiling to object not implemented");
    case Context::CompilationTarget::Assembly:
        // generate bitcode files from:
        // - sources, .ll
        // optimize bitcode files
        // generate asm files from:
        // - bitcode
        // output target
        // remove generated temp files
        fatalError("Compiling to assembly not implemented");
    }

    return EXIT_SUCCESS;
}

/**
 * Process provided input files from the context, resolve their path,
 * ansure they exost and store in driver paths structure
 */
void Driver::processInputs() {
    for (auto index = 0; index < Context::fileTypeCount; index++) {
        auto type = static_cast<Context::FileType>(index);
        auto& dst = getInputs(type);
        for (const auto& path : m_context.getInputFiles(type)) {
            auto resolved = m_context.resolveFilePath(path);
            dst.emplace_back(resolved);
        }
    }
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

void Driver::emitBitCode() {
    auto& bcFiles = getInputs(Context::FileType::BitCode);
    bcFiles.reserve(bcFiles.size() + m_modules.size());

    bool single = m_modules.size() == 1;
    for (auto& module : m_modules) {
        fs::path output = m_context.resolveOutputPath(module->getSourceFileName(), ".bc");

        std::error_code errors{};
        llvm::raw_fd_ostream stream{ output.string(), errors, llvm::sys::fs::OpenFlags::OF_None };
        llvm::WriteBitcodeToFile(*module, stream);
        stream.flush();
        stream.close();

        bcFiles.emplace_back(output);
    }
}

void Driver::emitObjects() {
    auto& bcFiles = getInputs(Context::FileType::BitCode);
    auto& objFiles = getInputs(Context::FileType::Object);
    objFiles.reserve(objFiles.size() + bcFiles.size());

    auto task = m_context.getToolchain().createTask(ToolKind::Assembler);
    task.reserve(4);
    for (const auto& input : bcFiles) {
        auto output = m_context.resolveOutputPath(input, ".obj");

        task.reset();
        task.addArg("-filetype=obj");
        task.addPath("-o", output);
        task.addPath(input.string());

        if (task.execute() != EXIT_SUCCESS) {
            fatalError("Failed emit '"s + output.string() + "'");
        }

        objFiles.emplace_back(output);
    }
}

void Driver::emitExecutable() {
    auto linker = m_context.getToolchain().createTask(ToolKind::Linker);
    auto objFiles = getInputs(Context::FileType::Object);
    auto output = m_context.getOutputPath();

    if (m_context.getTriple().isOSWindows()) {
        if (output == "") {
            output = m_context.resolveOutputPath(objFiles[0], "exe");
        }

        linker.addArg("-subsystem", "console");
        if (m_context.getTriple().isArch64Bit()) {
            auto sysLibPath = m_context.getToolchain().getBasePath() / "lib" / "win64";
            linker.addPath("-L", sysLibPath);
        } else {
            fatalError("Building 32bit not supported");
        }

        for (const auto& obj : objFiles) {
            linker.addPath(obj);
        }

        linker.addArg("-lmsvcrt");
    } else {
        fatalError("Compilation not this platform supported");
    }

    linker.addPath("-o", output);
    if (linker.execute() != EXIT_SUCCESS) {
        fatalError("Failed generate '"s + output.string() + "'");
    }
}

// Compile

void Driver::compileSources() {
    const auto& sources = getInputs(Context::FileType::Source);
    m_modules.reserve(m_modules.size() + sources.size());
    for (const auto& source : sources) {
        string included;
        auto ID = m_context.getSourceMrg().AddIncludeFile(source.string(), {}, included);
        if (ID == ~0U) {
            fatalError("Failed to load '"s + source.string() + "'");
        }

        compileSource(source, ID);
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
