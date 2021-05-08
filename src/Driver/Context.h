//
// Created by Albert Varaksin on 18/04/2021.
//
#pragma once
#include "pch.h"
#include "Toolchain/Toolchain.h"

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
    ~Context() = default;

    void validate() const noexcept;

    [[nodiscard]] CompilationTarget getCompilationTarget() const { return m_compilationTarget; }
    void setCompilationTarget(CompilationTarget target) { m_compilationTarget = target; }

    [[nodiscard]] OutputType getOutputType() const { return m_outputType; }
    void setOutputType(OutputType outputType) {
        m_outputType = outputType;
        if (m_compilationTarget == CompilationTarget::Executable) {
            setCompilationTarget(CompilationTarget::Assembly);
        }
    }

    [[nodiscard]] OptimizationLevel getOptimizationLevel() const { return m_optimizationLevel; }
    void setOptimizationLevel(OptimizationLevel level) { m_optimizationLevel = level; }

    [[nodiscard]] bool isDebugBuild() const { return m_isDebug; }
    void setDebugBuild(bool debug) { m_isDebug = debug; }

    void setVerbose(bool verbose) { m_verbose = verbose; }
    [[nodiscard]] bool isVerbose() const { return m_verbose; }

    void setImplicitMain(bool implicitMain) { m_implicitMain = implicitMain; }
    [[nodiscard]] bool getImplicitMain() const { return m_implicitMain; }

    void setMainFile(const fs::path& file) {
        if (file.extension() != getFileExt(FileType::Source)) {
            fatalError("main file must have '"_t + getFileExt(FileType::Source) + "' extension");
        }
        m_mainPath = file;
        m_implicitMain = true;
        addInputFile(file);
    }

    [[nodiscard]] const std::optional<fs::path>& getMainFile() const {
        return m_mainPath;
    }

    void addInputFile(const fs::path& path);
    [[nodiscard]] const std::vector<fs::path>& getInputFiles(FileType type) const;

    void setWorkingDir(const fs::path& path);
    [[nodiscard]] const fs::path& getWorkingDir() const { return m_workingDir; }

    void setCompilerPath(const fs::path& path);
    [[nodiscard]] const fs::path& getCompilerPath() const { return m_compilerPath; }
    [[nodiscard]] fs::path getCompilerDir() const { return m_compilerPath.parent_path(); }

    void setOutputFilePath(const fs::path& path);
    [[nodiscard]] const fs::path& getOutputPath() const { return m_outputFilePath; }

    [[nodiscard]] Toolchain& getToolchain() { return m_toolchain; }

    void setTriple(const llvm::Triple& triple) { m_triple = triple; }
    [[nodiscard]] llvm::Triple& getTriple() { return m_triple; }

    [[nodiscard]] llvm::SourceMgr& getSourceMrg() { return m_sourceMgr; }
    [[nodiscard]] llvm::LLVMContext& getLlvmContext() { return m_llvmContext; }

    [[nodiscard]] bool isTargetLinkable() const {
        return m_compilationTarget == CompilationTarget::Executable;
    }

    [[nodiscard]] bool isTargetNative() const {
        return m_compilationTarget == CompilationTarget::Executable;
    }

    [[nodiscard]] bool isOutputLLVMIr() const {
        return m_outputType == OutputType::LLVM && m_compilationTarget == CompilationTarget::Assembly;
    }

    /**
     * Return true if given source file should have an implicit main
     */
    [[nodiscard]] bool isMainFile(const fs::path& file) const;

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
     * Retain a copy of the string in the context and return a const StringRef& that
     * we can pass around safely without worry of it expiring (as long as context lives)
     * @param str string to retain
     * @return
     */
    [[nodiscard]] StringRef retainCopy(StringRef str) noexcept;

private:
    [[nodiscard]] static bool validateFile(const fs::path& path);

    bool m_verbose = false;
    OutputType m_outputType = OutputType::Native;
    CompilationTarget m_compilationTarget = CompilationTarget::Executable;
    OptimizationLevel m_optimizationLevel = OptimizationLevel::O2;
    bool m_isDebug = false;
    bool m_implicitMain = true;
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
};

} // namespace lbc
