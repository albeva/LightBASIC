//
// Created by Albert Varaksin on 13/07/2020.
//
#include "Driver.h"
#include "Gen/CodeGen.h"
#include "Parser/Parser.h"
#include "Sem/SemanticAnalyzer.h"
#include "TempFileCache.h"
#include "Toolchain/ToolTask.h"
#include "Toolchain/Toolchain.h"
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/FileSystem.h>

using namespace lbc;

Driver::Driver(Context& context)
: m_context{ context } {
}

int Driver::execute() {
    processInputs();
    compileSources();

    switch (m_context.getCompilationTarget()) {
    case Context::CompilationTarget::Executable:
        emitBitCode(true);
        emitObjects(true);
        emitExecutable();
        break;
    case Context::CompilationTarget::Object:
        switch (m_context.getOutputType()) {
        case Context::OutputType::Native:
            emitBitCode(true);
            emitObjects(false);
            break;
        case Context::OutputType::LLVM:
            emitBitCode(false);
            break;
        }
        break;
    case Context::CompilationTarget::Assembly:
        switch (m_context.getOutputType()) {
        case Context::OutputType::Native:
            emitBitCode(true);
            emitAssembly(false);
            break;
        case Context::OutputType::LLVM:
            emitLLVMIr(false);
            break;
        }
    }

    TempFileCache::removeTemporaryFiles();
    return EXIT_SUCCESS;
}

/**
 * Process provided input files from the context, resolve their path,
 * ansure they exost and store in driver paths structure
 */
void Driver::processInputs() {
    for (size_t index = 0; index < Context::fileTypeCount; index++) {
        auto type = static_cast<Context::FileType>(index);
        auto& dst = getArtifacts(type);
        for (const auto& path : m_context.getInputFiles(type)) {
            auto resolved = m_context.resolveFilePath(path);
            dst.emplace_back(type, dst.size(), resolved);
        }
    }
}

Driver::Artefact* Driver::findArtifact(Context::FileType type, const fs::path& path) {
    auto& inputs = getArtifacts(type);
    auto result = std::find_if(inputs.begin(), inputs.end(), [&](const Artefact& v) {
        return v.m_path == path;
    });
    if (result != inputs.end()) {
        return &*result;
    }
    return nullptr;
}

Driver::Artefact* Driver::findArtifact(const fs::path& path) {
    for (size_t index = 0; index < Context::fileTypeCount; index++) {
        if (auto* artifact = findArtifact(static_cast<Context::FileType>(index), path)) {
            return artifact;
        }
    }
    return nullptr;
}

void Driver::emitLLVMIr(bool temporary) {
    auto& irFiles = getArtifacts(Context::FileType::LLVMIr);
    irFiles.reserve(irFiles.size() + m_modules.size());

    for (auto& module : m_modules) {
        auto* file = findArtifact(module->getSourceFileName());
        assert(file != nullptr); // NOLINT
        auto output = temporary
            ? TempFileCache::createUniquePath(getOrigin(*file), ".ll")
            : m_context.resolveOutputPath(getOrigin(*file), ".ll");

        std::error_code errors{};
        llvm::raw_fd_ostream stream{
            output.string(),
            errors,
            llvm::sys::fs::OpenFlags::OF_None
        };

        auto* printer = llvm::createPrintModulePass(stream);
        printer->runOnModule(*module);

        stream.flush();
        stream.close();

        irFiles.emplace_back(file->m_origin, output);
    }
}

void Driver::emitBitCode(bool temporary) {
    auto& bcFiles = getArtifacts(Context::FileType::BitCode);
    bcFiles.reserve(bcFiles.size() + m_modules.size());

    for (auto& module : m_modules) {
        auto* file = findArtifact(module->getSourceFileName());
        assert(file != nullptr); // NOLINT
        auto output = temporary
            ? TempFileCache::createUniquePath(getOrigin(*file), ".bc")
            : m_context.resolveOutputPath(getOrigin(*file), ".bc");

        std::error_code errors{};
        llvm::raw_fd_ostream stream{
            output.string(),
            errors,
            llvm::sys::fs::OpenFlags::OF_None
        };
        llvm::WriteBitcodeToFile(*module, stream);
        stream.flush();
        stream.close();

        bcFiles.emplace_back(file->m_origin, output);
    }
}

void Driver::emitAssembly(bool temporary) {
    const auto& bcFiles = getArtifacts(Context::FileType::BitCode);
    auto& asmFiles = getArtifacts(Context::FileType::Assembly);
    asmFiles.reserve(asmFiles.size() + bcFiles.size());

    auto task = m_context.getToolchain().createTask(ToolKind::Assembler);
    task.reserve(4);
    for (const auto& input : bcFiles) {
        auto output = temporary
            ? TempFileCache::createUniquePath(getOrigin(input), ".s")
            : m_context.resolveOutputPath(getOrigin(input), ".s");

        task.reset();
        task.addArg("-filetype=asm");
        task.addPath("-o", output);
        task.addPath(input.m_path.string());

        if (task.execute() != EXIT_SUCCESS) {
            fatalError("Failed emit '"s + output.string() + "'");
        }

        asmFiles.emplace_back(input.m_origin, output);
    }
}

void Driver::emitObjects(bool temporary) {
    const auto& bcFiles = getArtifacts(Context::FileType::BitCode);
    auto& objFiles = getArtifacts(Context::FileType::Object);
    objFiles.reserve(objFiles.size() + bcFiles.size());

    auto task = m_context.getToolchain().createTask(ToolKind::Assembler);
    task.reserve(4);
    for (const auto& input : bcFiles) {
        auto output = temporary
            ? TempFileCache::createUniquePath(getOrigin(input), ".obj")
            : m_context.resolveOutputPath(getOrigin(input), ".obj");

        task.reset();
        task.addArg("-filetype=obj");
        task.addPath("-o", output);
        task.addPath(input.m_path.string());

        if (task.execute() != EXIT_SUCCESS) {
            fatalError("Failed emit '"s + output.string() + "'");
        }

        objFiles.emplace_back(input.m_origin, output);
    }
}

void Driver::emitExecutable() {
    auto linker = m_context.getToolchain().createTask(ToolKind::Linker);
    auto objFiles = getArtifacts(Context::FileType::Object);
    auto output = m_context.getOutputPath();
    const auto& triple = m_context.getTriple();

    if (triple.isArch32Bit()) {
        fatalError("32bit is not implemented yet");
    }

    if (triple.isOSWindows()) {
        if (output.empty()) {
            const auto& sources = m_context.getInputFiles(Context::FileType::Source);
            if (sources.empty()) {
                output = m_context.getWorkingDir() / "a.exe";
            } else {
                output = m_context.getWorkingDir() / sources[0].stem().replace_extension("exe");
            }
        }

        auto sysLibPath = m_context.getToolchain().getBasePath() / "lib" / "win64";
        linker
            .addArg("-subsystem", "console")
            .addPath("-L", sysLibPath);

        for (const auto& obj : objFiles) {
            linker.addPath(obj.m_path);
        }

        linker.addArg("-lmsvcrt");
    } else if (triple.isMacOSX()) {
        if (output.empty()) {
            output = m_context.getWorkingDir() / "a.out";
        }

        linker
            .addPath("-L", "/usr/local/lib")
            .addPath("-syslibroot", "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
            .addArg("-lSystem")
            .addPath("-o", output);

        for (const auto& obj : objFiles) {
            linker.addPath(obj.m_path);
        }
    } else if (triple.isOSLinux()) {
        if (output.empty()) {
            output = m_context.getWorkingDir() / "a.out";
        }

        string linuxSysPath = "/usr/lib/x86_64-linux-gnu";
        linker
            .addArg("-m", "elf_x86_64")
            .addArg("-dynamic-linker", "/lib64/ld-linux-x86-64.so.2")
            .addArg("-L", "/usr/lib")
            .addArg(linuxSysPath + "/crt1.o")
            .addArg(linuxSysPath + "/crti.o")
            .addPath("-o", output);

        for (const auto& obj : objFiles) {
            linker.addPath(obj.m_path);
        }

        linker.addArg("-lc");
        linker.addArg(linuxSysPath + "/crtn.o");
    } else {
        fatalError("Compilation not this platform not supported");
    }

    if (linker.execute() != EXIT_SUCCESS) {
        fatalError("Failed generate '"s + output.string() + "'");
    }
}

// Compile

void Driver::compileSources() {
    const auto& sources = getArtifacts(Context::FileType::Source);
    m_modules.reserve(m_modules.size() + sources.size());
    for (const auto& source : sources) {
        string included;
        auto ID = m_context.getSourceMrg().AddIncludeFile(source.m_path.string(), {}, included);
        if (ID == ~0U) {
            fatalError("Failed to load '"s + source.m_path.string() + "'");
        }
        compileSource(source.m_path, ID);
    }
}

void Driver::compileSource(const fs::path& path, unsigned int ID) {
    if (m_context.isVerbose()) {
        std::cout << "Compile: " << path << '\n';
    }

    bool isMain = m_context.isMainFile(path);
    Parser parser{ m_context, ID, isMain };
    auto ast = parser.parse();
    if (!ast) {
        fatalError("Failed to parse '"s + path.string() + "'");
    }

    // Analyze
    SemanticAnalyzer sem{ m_context };
    sem.visit(ast.get());

    // generate IR
    CodeGen gen{ m_context };
    gen.visit(ast.get());

    // done
    if (!gen.validate()) {
        fatalError("Failed to compile '"s + path.string() + "'");
    }

    // Happy Days
    m_modules.emplace_back(gen.getModule());
}
