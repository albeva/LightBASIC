//
// Created by Albert Varaksin on 13/07/2020.
//
#include "Driver.h"
#include "Ast/AstPrinter.h"
#include "Ast/CodePrinter.h"
#include "Gen/CodeGen.h"
#include "Parser/Parser.h"
#include "Sem/SemanticAnalyzer.h"
#include "TempFileCache.h"
#include "Toolchain/ToolTask.h"
#include "Toolchain/Toolchain.h"
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Support/FileSystem.h>

using namespace lbc;

void Driver::drive() noexcept {
    processInputs();
    compileSources();

    if (m_context.getDumpAst()) {
        dumpAst();
        return;
    }

    if (m_context.getDumpCode()) {
        dumpCode();
        return;
    }

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
}

/**
 * Process provided input files from the context, resolve their path,
 * ansure they exost and store in driver paths structure
 */
void Driver::processInputs() noexcept {
    for (size_t index = 0; index < Context::fileTypeCount; index++) {
        auto type = static_cast<Context::FileType>(index);
        auto& dst = getSources(type);
        for (const auto& path : m_context.getInputFiles(type)) {
            dst.emplace_back(Source::create(type, m_context.resolveFilePath(path), false));
        }
    }
}

std::unique_ptr<Source> Driver::deriveSource(const Source& source, Context::FileType type, bool temporary) const noexcept {
    const auto& original = source.origin.path;
    const auto ext = Context::getFileExt(type);
    const auto path = temporary
        ? TempFileCache::createUniquePath(original, ext)
        : m_context.resolveOutputPath(original, ext);
    return source.derive(type, path);
}

void Driver::emitLLVMIr(bool temporary) noexcept {
    emitLlvm(Context::FileType::LLVMIr, temporary, [](auto& stream, auto& module) {
        auto* printer = llvm::createPrintModulePass(stream);
        printer->runOnModule(module);
    });
}

void Driver::emitBitCode(bool temporary) noexcept {
    emitLlvm(Context::FileType::BitCode, temporary, [](auto& stream, auto& module) {
        llvm::WriteBitcodeToFile(module, stream);
    });
}

void Driver::emitLlvm(Context::FileType type, bool temporary, void (*generator)(llvm::raw_fd_ostream&, llvm::Module&)) noexcept {
    auto& dstFiles = getSources(type);
    dstFiles.reserve(dstFiles.size() + m_modules.size());

    for (auto& module : m_modules) {
        const auto& source = module->source;
        auto output = deriveSource(*source, type, temporary);

        std::error_code errors{};
        llvm::raw_fd_ostream stream{
            output->path.string(),
            errors,
            llvm::sys::fs::OpenFlags::OF_None
        };

        generator(stream, *module->llvmModule);

        stream.flush();
        stream.close();

        dstFiles.emplace_back(std::move(output));
    }
}

void Driver::emitAssembly(bool temporary) noexcept {
    emitNative(Context::FileType::Assembly, temporary);
}

void Driver::emitObjects(bool temporary) noexcept {
    emitNative(Context::FileType::Object, temporary);
}

void Driver::emitNative(Context::FileType type, bool temporary) noexcept {
    const auto& bcFiles = getSources(Context::FileType::BitCode);
    auto& dstFiles = getSources(type);
    string filetype;
    if (type == Context::FileType::Object) {
        filetype = "obj";
    } else {
        filetype = "asm";
    }
    dstFiles.reserve(dstFiles.size() + bcFiles.size());

    auto assembler = m_context.getToolchain().createTask(ToolKind::Assembler);
    for (const auto& source : bcFiles) {
        auto output = deriveSource(*source, type, temporary);

        assembler.reset();
        assembler.addArg("-filetype="s + filetype);
        assembler.addPath("-o", output->path);
        assembler.addPath(source->path);

        if (assembler.execute() != EXIT_SUCCESS) {
            fatalError("Failed emit '"_t + output->path.string() + "'");
        }

        dstFiles.emplace_back(std::move(output));
    }
}

void Driver::emitExecutable() noexcept {
    auto linker = m_context.getToolchain().createTask(ToolKind::Linker);
    const auto& objFiles = getSources(Context::FileType::Object);
    const auto& triple = m_context.getTriple();

    if (objFiles.empty()) {
        fatalError("No objects to link");
    }

    if (!triple.isX86()) {
        fatalError("Currently only x86 is supported");
    }

    if (triple.isArch32Bit()) {
        fatalError("32bit is not implemented yet");
    }

    auto output = m_context.getOutputPath();
    if (output.empty()) {
        output = m_context.getWorkingDir() / objFiles[0]->origin.path.stem();
        if (triple.isOSWindows()) {
            output += ".exe";
        }
    } else if (output.is_relative()) {
        output = fs::absolute(m_context.getWorkingDir() / output);
    }

    if (triple.isOSWindows()) {
        auto sysLibPath = m_context.getToolchain().getBasePath() / "lib" / "win64";
        linker
            .addArg("-m", "i386pep")
            .addPath("-o", output)
            .addArg("-subsystem", "console")
            .addArg("-s")
            .addPath("-L", sysLibPath)
            .addPath(sysLibPath / "crt2.o")
            .addPath(sysLibPath / "crtbegin.o");

        for (const auto& obj : objFiles) {
            linker.addPath(obj->path);
        }

        linker
            .addArgs({ "-(",
                "-lgcc",
                "-lmsvcrt",
                "-lkernel32",
                "-luser32",
                "-lmingw32",
                "-lmingwex",
                "-)" })
            .addPath(sysLibPath / "crtend.o");
    } else if (triple.isMacOSX()) {
        linker
            .addPath("-L", "/usr/local/lib")
            .addPath("-syslibroot", "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk")
            .addArg("-lSystem")
            .addPath("-o", output);

        for (const auto& obj : objFiles) {
            linker.addPath(obj->path);
        }
    } else if (triple.isOSLinux()) {
        string linuxSysPath = "/usr/lib/x86_64-linux-gnu";
        linker
            .addArg("-m", "elf_x86_64")
            .addArg("-dynamic-linker", "/lib64/ld-linux-x86-64.so.2")
            .addArg("-L", "/usr/lib")
            .addArg(linuxSysPath + "/crt1.o")
            .addArg(linuxSysPath + "/crti.o")
            .addPath("-o", output);

        for (const auto& obj : objFiles) {
            linker.addPath(obj->path);
        }

        linker.addArg("-lc");
        linker.addArg(linuxSysPath + "/crtn.o");
    } else {
        fatalError("Compilation not this platform not supported");
    }

    if (linker.execute() != EXIT_SUCCESS) {
        fatalError("Failed generate '"_t + output.string() + "'");
    }
}

// Compile

void Driver::compileSources() noexcept {
    const auto& sources = getSources(Context::FileType::Source);
    m_modules.reserve(m_modules.size() + sources.size());
    for (const auto& source : sources) {
        string included;
        auto ID = m_context.getSourceMrg().AddIncludeFile(source->path.string(), {}, included);
        if (ID == ~0U) {
            fatalError("Failed to load '"_t + source->path.string() + "'");
        }
        compileSource(source.get(), ID);
    }
}

void Driver::compileSource(const Source* source, unsigned int ID) noexcept {
    const auto& path = source->path;
    if (m_context.isVerbose()) {
        std::cout << "Compile: " << path << '\n';
    }

    bool isMain = m_context.isMainFile(path);
    Parser parser{ m_context, ID, isMain };
    auto ast = parser.parse();
    if (!ast) {
        fatalError("Failed to parse '"_t + path.string() + "'");
    }

    // Analyze
    SemanticAnalyzer sem{ m_context };
    sem.visit(ast.get());

    if (m_context.getDumpAst() || m_context.getDumpCode()) {
        m_modules.emplace_back(std::make_unique<TranslationUnit>(
            nullptr,
            source,
            std::move(ast)));
        return;
    }

    // generate IR
    CodeGen gen{ m_context };
    gen.visit(ast.get());

    // done
    if (!gen.validate()) {
        fatalError("Failed to compile '"_t + path.string() + "'");
    }

    // Happy Days
    m_modules.emplace_back(std::make_unique<TranslationUnit>(
        gen.getModule(),
        source,
        std::move(ast)));
}

void Driver::dumpAst() noexcept {
    auto print = [&](llvm::raw_ostream& stream) {
        AstPrinter printer{ m_context, stream };
        for (const auto& module : m_modules) {
            printer.visit(module->ast.get());
        }
    };

    auto output = m_context.getOutputPath();
    if (output.empty()) {
        print(llvm::outs());
    } else {
        if (output.is_relative()) {
            output = fs::absolute(m_context.getWorkingDir() / output);
        }

        std::error_code errors{};
        llvm::raw_fd_ostream stream{
            output.string(),
            errors,
            llvm::sys::fs::OpenFlags::OF_None
        };

        print(stream);

        stream.flush();
        stream.close();
    }
}

void Driver::dumpCode() {
    auto print = [&](llvm::raw_ostream& stream) {
        CodePrinter printer{ stream };
        for (const auto& module : m_modules) {
            printer.visit(module->ast.get());
        }
    };

    auto output = m_context.getOutputPath();
    if (output.empty()) {
        print(llvm::outs());
    } else {
        if (output.is_relative()) {
            output = fs::absolute(m_context.getWorkingDir() / output);
        }

        std::error_code errors{};
        llvm::raw_fd_ostream stream{
            output.string(),
            errors,
            llvm::sys::fs::OpenFlags::OF_None
        };

        print(stream);

        stream.flush();
        stream.close();
    }
}
