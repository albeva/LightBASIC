//
// Created by Albert on 13/07/2020.
//
#include "Driver.h"
#include "Ast/Ast.h"
#include "Gen/CodeGen.h"
#include "Parser/Parser.h"
#include "Sem/SemanticAnalyzer.h"
#include <llvm/IR/IRPrintingPasses.h>
using namespace lbc;

Driver::Driver()
: m_triple{ llvm::sys::getDefaultTargetTriple() } {
}

Driver::~Driver() = default;

[[noreturn]] static void error(const string& message) {
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
}

// Process input

int Driver::execute() {
    compileSources();

    switch (m_result) {
    case CompileResult::Default:
        emitExecutable();
        break;
    case CompileResult::LLVMIr:
        emitLLVMIr();
        break;
    case CompileResult::BitCode:
        emitBitCode(true);
        break;
    case CompileResult::Assembly:
        emitNative(CompileResult::Assembly, true);
        break;
    case CompileResult::Object:
        emitNative(CompileResult::Object, true);
        break;
    case CompileResult::Executable:
        emitExecutable();
        break;
    case CompileResult::Library:
        error("Creating libraries is not supported");
    }

    return EXIT_SUCCESS;
}

void Driver::emitLLVMIr() {
    if (m_level > OptimizationLevel::O0) {
        auto bcFiles = emitBitCode(false);
        optimize(bcFiles, CompileResult::LLVMIr, true);
        for (const auto& file : bcFiles) {
            fs::remove(file);
        }
    } else {
        bool single = m_modules.size() == 1;
        for (auto& module : m_modules) {
            fs::path path = resolveOutputPath(module->getSourceFileName(), ".ll", single, true);

            std::error_code ec{};
            llvm::raw_fd_ostream os{ path.string(), ec, llvm::sys::fs::OF_Text };

            auto* printer = llvm::createPrintModulePass(os);
            printer->runOnModule(*module);

            os.flush();
            os.close();
        }
    }
}

std::vector<fs::path> Driver::emitBitCode(bool final) {
    std::vector<fs::path> result;
    result.reserve(m_modules.size());

    bool single = m_modules.size() == 1;
    for (auto& module : m_modules) {
        fs::path output = resolveOutputPath(module->getSourceFileName(), ".bc", single, final);

        std::error_code errors{};
        llvm::raw_fd_ostream stream{ output.string(), errors, llvm::sys::fs::OF_None };
        llvm::WriteBitcodeToFile(*module, stream);
        stream.flush();
        stream.close();

        result.emplace_back(output);
    }

    return result;
}

std::vector<fs::path> Driver::emitNative(CompileResult emit, bool final) {
    string fileType;
    string ext;
    switch (emit) {
    case CompileResult::Object:
        fileType = "-filetype=obj";
        ext = ".o";
        break;
    case CompileResult::Assembly:
        fileType = "-filetype=asm";
        ext = ".s";
        break;
    default:
        llvm_unreachable("Emit Native only emits object and asm");
    }

    std::vector<fs::path> result{};
    result.reserve(m_modules.size());

    auto bcFiles = emitBitCode(false);
    if (m_level > OptimizationLevel::O0) {
        bcFiles = optimize(bcFiles, CompileResult::BitCode, false);
    }

    auto tool = getToolPath(Tool::Assembler).string();
    bool single = bcFiles.size() == 1;
    for (const auto& path : bcFiles) {
        string input{ path.string() };
        auto output = resolveOutputPath(path, ext, single, final).string();

        std::vector<llvm::StringRef> args{
            tool,
            fileType,
            "-o",
            output,
            input
        };
        auto code = llvm::sys::ExecuteAndWait(tool, args);
        if (code != EXIT_SUCCESS) {
            error("Failed emit '"s + output + "'");
        }
        result.emplace_back(output);

        fs::remove(input);
    }

    return result;
}

void Driver::emitExecutable() {
    auto objects = emitNative(CompileResult::Object, false);
    auto output = resolveOutputPath("a", ".out", true, true).string();
    auto tool = getToolPath(Tool::Linker).string();

    std::vector<string> stringCopies;
    stringCopies.reserve(objects.size());

    std::vector<llvm::StringRef> args;
    constexpr auto reserve = 16;
    args.reserve(objects.size() + reserve);
    args.emplace_back(tool);

    fs::path linuxSysPath;
    if (m_triple.isOSLinux()) {
        if (m_triple.isArch32Bit()) {
            args.emplace_back("-m");
            args.emplace_back("elf_i386");
            linuxSysPath = "/usr/libr32";
            if (!fs::exists(linuxSysPath)) {
                linuxSysPath = "/usr/lib/i386-linux-gnu";
                if (!fs::exists(linuxSysPath)) {
                    error("No 32 bit libraries found");
                }
            }
        } else if (m_triple.isArch64Bit()) {
            args.emplace_back("-m");
            args.emplace_back("elf_x86_64");
            args.emplace_back("-dynamic-linker");
            args.emplace_back("/lib64/ld-linux-x86-64.so.2");
            linuxSysPath = "/usr/lib/x86_64-linux-gnu";
        } else {
            error("Unknown architecture");
        }
        args.emplace_back("-L");
        args.emplace_back("/usr/lib");

        args.emplace_back(stringCopies.emplace_back((linuxSysPath / "crt1.o").string()));
        args.emplace_back(stringCopies.emplace_back((linuxSysPath / "crti.o").string()));
    } else if (m_triple.isMacOSX()) {
        args.emplace_back("-lSystem");
    } else if (m_triple.isOSWindows()) {
        error("Building for Windows not currently supported");
    }
    args.emplace_back("-o");
    args.emplace_back(output);

    for (const auto& path : objects) {
        args.emplace_back(stringCopies.emplace_back(path.string()));
    }

    if (m_triple.isOSLinux()) {
        args.emplace_back("--no-as-needed");
        args.emplace_back("-lc");
        args.emplace_back(stringCopies.emplace_back((linuxSysPath / "crtn.o").string()));
    }

    auto code = llvm::sys::ExecuteAndWait(tool, args);
    if (code != EXIT_SUCCESS) {
        error("Failed generate '"s + output + "'");
    }

    for (const auto& path : objects) {
        fs::remove(path);
    }
}

// Compile

void Driver::compileSources() {
    const auto& sources = getResources(ResourceType::Source);
    m_modules.reserve(sources.size());
    for (const auto& source : sources) {
        auto path = resolvePath(source, ResourceType::SourceDirectory).string();

        string included;
        auto ID = m_sourceMgr.AddIncludeFile(path, {}, included);
        if (ID == ~0U) {
            error("Failed to load '"s + path + "'");
        }

        compileSource(path, ID);
    }
}

void Driver::compileSource(const fs::path& path, unsigned int ID) {
    Parser parser{ m_sourceMgr, ID };
    auto ast = parser.parse();
    if (!ast) {
        error("Failed to parse '"s + path.string() + "'");
    }

    // Analyze
    SemanticAnalyzer sem(m_llvmContext, m_sourceMgr, ID);
    ast->accept(&sem);

    // generate IR
    CodeGen gen(m_llvmContext, m_sourceMgr, m_triple, ID);
    ast->accept(&gen);

    // done
    if (!gen.validate()) {
        error("Failed to compile '"s + path.string() + "'");
    }

    // Happy Days
    m_modules.emplace_back(gen.getModule());
}

// Optimize

std::vector<fs::path> Driver::optimize(const std::vector<fs::path>& files, CompileResult emit, bool final) {
    std::vector<fs::path> result;
    result.reserve(files.size());

    string emitOpt;
    string ext;
    switch (emit) {
    case CompileResult::LLVMIr:
        emitOpt = "-S";
        ext = ".ll";
        break;
    case CompileResult::BitCode:
        emitOpt = "";
        ext = ".bc";
        break;
    case CompileResult::Assembly:
        emitOpt = "--filetype=asm";
        ext = ".asm";
        break;
    case CompileResult::Object:
        emitOpt = "--filetype=obj";
        ext = ".o";
        break;
    case CompileResult::Default:
    case CompileResult::Executable:
    case CompileResult::Library:
        llvm_unreachable("Optimizer does not generate executable/library output");
    }

    auto level = getCmdOption(m_level);
    auto tool = getToolPath(Tool::Optimizer).string();
    bool single = files.size() == 1;
    for (const auto& path : files) {
        string file{ path.string() };
        auto output = resolveOutputPath(path, ext, single, final).string();
        std::vector<llvm::StringRef> args{
            tool,
            level,
            "-o",
            output,
            file
        };
        if (!emitOpt.empty()) {
            args.insert(std::next(args.begin()), emitOpt);
        }
        auto code = llvm::sys::ExecuteAndWait(tool, args);
        if (code != EXIT_SUCCESS) {
            error("Failed to optimize '"s + file + "'");
        }

        result.emplace_back(output);
    }

    return result;
}

// Path management

fs::path Driver::resolveOutputPath(const fs::path& path, const string& ext, bool single, bool final) const {
    if (m_outputPath.empty()) {
        fs::path output{ path };
        output.replace_extension(ext);
        if (!output.is_absolute()) {
            output = fs::absolute(m_workingDir / output);
        }
        fs::create_directories(output.parent_path());
        return output;
    }

    if (fs::is_directory(m_outputPath) || m_outputPath.filename().empty()) {
        fs::path relative = fs::relative(path, m_workingDir);
        relative.replace_extension(ext);
        auto fullPath = fs::absolute(m_outputPath / relative);
        fs::create_directories(fullPath.parent_path());
        return fullPath;
    }

    if (!final) {
        fs::path relative = fs::relative(path, m_workingDir);
        relative.replace_extension(ext);
        auto fullPath = fs::absolute(m_workingDir / relative);
        fs::create_directories(fullPath.parent_path());
        return fullPath;
    }

    if (!single) {
        error("Output path '"s + m_outputPath.string() + "' can have only one input file");
    }

    fs::create_directories(m_outputPath.parent_path());
    return m_outputPath;
}

fs::path Driver::resolvePath(const fs::path& path, ResourceType type) const {
    if (path.is_absolute()) {
        if (validateFile(path, true)) {
            return path;
        }
    }

    for (const auto& dir : getResources(type)) {
        auto p = fs::absolute(dir / path);
        if (validateFile(p, false)) {
            return p;
        }
    }

    if (validateFile(path, true)) {
        return path;
    }

    llvm_unreachable("file resolving failed");
}

[[nodiscard]] bool Driver::validateFile(const fs::path& path, bool mustExist) {
    if (!fs::exists(path)) {
        if (mustExist) {
            error("File '"s + path.string() + "' not found");
        }
        return false;
    }

    if (!fs::is_regular_file(path)) {
        error("File '"s + path.string() + "' is not regular");
    }

    return true;
}

void Driver::setWorkingDir(const fs::path& path) {
    if (!path.is_absolute()) {
        error("Working dir not a full path");
    }

    if (!fs::exists(path)) {
        error("Working dir does not exist");
    }

    if (!fs::is_directory(path)) {
        error("Working dir must point to a directory");
    }

    m_workingDir = path;
    addResource(ResourceType::SourceDirectory, path);
    addResource(ResourceType::LibraryDirectory, path);
}

void Driver::setOutputPath(const fs::path& path) {
    if (path.is_absolute()) {
        m_outputPath = path;
    } else {
        m_outputPath = fs::absolute(m_workingDir / path);
    }

    if (m_result == CompileResult::Default) {
        auto ext = m_outputPath.extension();
        if (ext == ".o") {
            m_result = CompileResult::Object;
        } else if (ext == ".ll") {
            m_result = CompileResult::LLVMIr;
        } else if (ext == ".bc") {
            m_result = CompileResult::BitCode;
        } else if (ext == ".s") {
            m_result = CompileResult::Assembly;
        }
    }
}

// Manage resources

void Driver::addResource(Driver::ResourceType type, const fs::path& path) {
    auto index = static_cast<size_t>(type);
    m_resources.at(index).insert(path);
}

const Driver::ResourceContainer& Driver::getResources(Driver::ResourceType type) const {
    auto index = static_cast<size_t>(type);
    return m_resources.at(index);
}

// Manage tools

fs::path Driver::getToolPath(Tool tool) {
    fs::path path;
    switch (tool) {
    case Tool::Optimizer:
        path = "/usr/local/bin/opt";
        break;
    case Tool::Assembler:
        path = "/usr/local/bin/llc";
        break;
    case Tool::Linker:
        path = "/usr/bin/ld";
        break;
    default:
        llvm_unreachable("Invalid Tool ID");
    }
    if (!fs::exists(path)) {
        error("Tool "s + path.string() + " not found!");
    }
    return path;
}

// Stringify

string Driver::getCmdOption(Driver::OptimizationLevel level) {
    switch (level) {
    case OptimizationLevel::O0:
        return "-O0";
    case OptimizationLevel::O1:
        return "-O1";
    case OptimizationLevel::O2:
        return "-O2";
    case OptimizationLevel::O3:
        return "-O3";
    }
    llvm_unreachable("Unreachable optimization level");
}
