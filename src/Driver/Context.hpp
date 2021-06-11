//
// Created by Albert Varaksin on 18/04/2021.
//
#pragma once
#include "Driver/Toolchain/Toolchain.hpp"

namespace lbc {

/**
 * Hold information about compilation process
 * configuration, input and expected outputs
 */
class Context final {
public:
    NO_COPY_AND_MOVE(Context)

    enum class CompilationTarget {
        Executable,
        Object,
        Assembly
    };

    enum class OutputType {
        Native,
        LLVM
    };

    enum class OptimizationLevel {
        O0,
        OS,
        O1,
        O2,
        O3
    };

    enum class FileType {
        Source,   // anything, but mostly .bas
        Assembly, // .s
        Object,   // .o
        LLVMIr,   // .ll
        BitCode   // .bc
    };
    static constexpr size_t fileTypeCount = 5;

    [[nodiscard]] static string getFileExt(FileType type);

    Context();
    ~Context() noexcept = default;

    void validate() const noexcept;

    [[nodiscard]] CompilationTarget getCompilationTarget() const noexcept { return m_compilationTarget; }
    void setCompilationTarget(CompilationTarget target) noexcept { m_compilationTarget = target; }

    [[nodiscard]] OutputType getOutputType() const noexcept { return m_outputType; }
    void setOutputType(OutputType outputType) noexcept {
        m_outputType = outputType;
        if (m_compilationTarget == CompilationTarget::Executable) {
            setCompilationTarget(CompilationTarget::Assembly);
        }
    }

    [[nodiscard]] bool getDumpAst() const noexcept { return m_astDump; }
    void setDumpAst(bool dump) noexcept { m_astDump = dump; }

    [[nodiscard]] bool getDumpCode() const noexcept { return m_codeDump; }
    void setDumpCode(bool dump) noexcept { m_codeDump = dump; }

    [[nodiscard]] OptimizationLevel getOptimizationLevel() const noexcept { return m_optimizationLevel; }
    void setOptimizationLevel(OptimizationLevel level) noexcept { m_optimizationLevel = level; }

    [[nodiscard]] bool isDebugBuild() const noexcept { return m_isDebug; }
    void setDebugBuild(bool debug) noexcept { m_isDebug = debug; }

    [[nodiscard]] bool isVerbose() const noexcept { return m_verbose; }
    void setVerbose(bool verbose) noexcept { m_verbose = verbose; }

    [[nodiscard]] bool getImplicitMain() const noexcept { return m_implicitMain; }
    void setImplicitMain(bool implicitMain) noexcept { m_implicitMain = implicitMain; }

    void setMainFile(const fs::path& file) {
        if (file.extension() != getFileExt(FileType::Source)) {
            fatalError("main file must have '"_t + getFileExt(FileType::Source) + "' extension");
        }
        m_mainPath = file;
        m_implicitMain = true;
        addInputFile(file);
    }

    [[nodiscard]] const std::optional<fs::path>& getMainFile() const noexcept {
        return m_mainPath;
    }

    void addInputFile(const fs::path& path);
    [[nodiscard]] const std::vector<fs::path>& getInputFiles(FileType type) const noexcept;

    [[nodiscard]] const fs::path& getWorkingDir() const noexcept { return m_workingDir; }
    void setWorkingDir(const fs::path& path);

    [[nodiscard]] const fs::path& getCompilerPath() const noexcept { return m_compilerPath; }
    [[nodiscard]] fs::path getCompilerDir() const noexcept { return m_compilerPath.parent_path(); }
    void setCompilerPath(const fs::path& path);

    [[nodiscard]] const fs::path& getOutputPath() const noexcept { return m_outputFilePath; }
    void setOutputFilePath(const fs::path& path);

    [[nodiscard]] Toolchain& getToolchain() noexcept { return m_toolchain; }

    [[nodiscard]] llvm::Triple& getTriple() noexcept { return m_triple; }
    void setTriple(const llvm::Triple& triple) noexcept { m_triple = triple; }

    [[nodiscard]] llvm::SourceMgr& getSourceMrg() noexcept { return m_sourceMgr; }
    [[nodiscard]] llvm::LLVMContext& getLlvmContext() noexcept { return m_llvmContext; }

    [[nodiscard]] bool isTargetLinkable() const noexcept {
        return m_compilationTarget == CompilationTarget::Executable;
    }

    [[nodiscard]] bool isTargetNative() const noexcept {
        return m_compilationTarget == CompilationTarget::Executable;
    }

    [[nodiscard]] bool isOutputLLVMIr() const noexcept {
        return m_outputType == OutputType::LLVM && m_compilationTarget == CompilationTarget::Assembly;
    }

    /**
     * Return true if given source file should have an implicit main
     */
    [[nodiscard]] bool isMainFile(const fs::path& file) const noexcept;

    /**
     * Resolve input file path to corresponding output path.
     *
     * @param path input file that exists and needs output to be mapped
     * @param ext new extension for the output file
     * @return final path for the output path
     */
    [[nodiscard]] fs::path resolveOutputPath(const fs::path& path, const string& ext) const;

    /**
     * Resolve input type taking into account currently defined paths
     */
    [[nodiscard]] fs::path resolveFilePath(const fs::path& path) const;

    /**
     * Retain a copy of the string in the context and return a StringRef that
     * we can pass around safely without worry of it expiring (as long as context lives)
     * @param str string to retain
     */
    [[nodiscard]] StringRef retainCopy(StringRef str);

    /**
     * Store imported modules
     * @param module
     * @return true if module is newly added, false otherwise
     */
    [[nodiscard]] bool import(StringRef module);

private:
    [[nodiscard]] static bool validateFile(const fs::path& path);

    bool m_verbose = false;
    OutputType m_outputType = OutputType::Native;
    CompilationTarget m_compilationTarget = CompilationTarget::Executable;
    OptimizationLevel m_optimizationLevel = OptimizationLevel::O2;
    bool m_isDebug = false;
    bool m_implicitMain = true;
    bool m_astDump = false;
    bool m_codeDump = false;
    std::optional<fs::path> m_mainPath{};

    std::array<std::vector<fs::path>, fileTypeCount> m_inputFiles{};

    Toolchain m_toolchain{ *this };
    fs::path m_workingDir;
    fs::path m_compilerPath{};
    fs::path m_outputFilePath{};

    llvm::Triple m_triple;
    llvm::SourceMgr m_sourceMgr{};
    llvm::LLVMContext m_llvmContext{};

    llvm::StringSet<> m_retainedStrings{};
    llvm::StringSet<> m_imports;
};

} // namespace lbc
