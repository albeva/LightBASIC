//
// Created by Albert Varaksin on 18/04/2021.
//
#pragma once
#include "Toolchain/Toolchain.h"

namespace lbc {

/**
 * Hold information about compilation process
 * configuration, input and expected outputs
 */
class Context final {
    NON_COPYABLE(Context)
public:
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
    ~Context();

    void validate() const noexcept;

    [[nodiscard]] CompilationTarget getCompilationTarget() const { return m_compilationTarget; }
    void setCompilationTarget(CompilationTarget target) { m_compilationTarget = target; }

    [[nodiscard]] OutputType getOutputType() const { return m_outputType; }
    void setOutputType(OutputType outputType) { m_outputType = outputType; }

    [[nodiscard]] OptimizationLevel getOptimizationLevel() const { return m_optimizationLevel; }
    void setOptimizationLevel(OptimizationLevel level) { m_optimizationLevel = level; }

    [[nodiscard]] bool isDebugBuild() const { return m_isDebug; }
    void setDebugBuild(bool debug) { m_isDebug = debug; }

    void setVerbose(bool verbose) { m_verbose = verbose; }
    [[nodiscard]] bool getVerbose() const { return m_verbose; }

    void addInputFile(const fs::path& path);
    [[nodiscard]] const std::vector<fs::path>& getInputFiles(FileType type) const;

    void setWorkingDir(const fs::path& path);
    [[nodiscard]] const fs::path& getWorkingDir() const { return m_workingDir; }

    void setCompilerPath(const fs::path& path) { m_compilerPath = path; }
    [[nodiscard]] const fs::path& getCompilerPath() const { return m_compilerPath; }

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
     * Validate that file exists and is a valid readable file
     *
     * @param path to the file to validate
     * @param mustExist if file does not exist lbc will quit with an error
     * @return true if file is valid
     */
    [[nodiscard]] static bool validateFile(const fs::path& path, bool mustExist);

    /**
     * Resolve input file path to corresponding output path.
     *
     * This will create any missing directories.
     *
     * @param path input file that exists and needs output to be mapped
     * @param ext favoured extension for the output
     * @param single is this a single file being emitted?
     * @param final is this final output, or intermediary
     * @return final path for the output path
     */
    [[nodiscard]] fs::path resolveOutputPath(const fs::path& path, const string& ext, bool single, bool final) const;

    /**
     * Resolve input type taking into account currently defined paths
     */
    [[nodiscard]] fs::path resolvePath(const fs::path& path) const;

private:
    bool m_verbose = false;
    OutputType m_outputType = OutputType::Native;
    CompilationTarget m_compilationTarget = CompilationTarget::Executable;
    OptimizationLevel m_optimizationLevel = OptimizationLevel::O2;
    bool m_isDebug = false;

    std::array<std::vector<fs::path>, fileTypeCount> m_inputFiles{};

    Toolchain m_toolchain{};
    fs::path m_workingDir{};
    fs::path m_compilerPath{};
    fs::path m_outputFilePath{};

    llvm::Triple m_triple;
    llvm::SourceMgr m_sourceMgr{};
    llvm::LLVMContext m_llvmContext{};
};

} // namespace lbc
