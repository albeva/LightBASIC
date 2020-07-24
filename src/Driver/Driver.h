//
// Created by Albert on 13/07/2020.
//
#include "pch.h"

namespace lbc {

/**
 * Drive compilation process
 */
class Driver final {
    NON_COPYABLE(Driver)
public:
    Driver();
    ~Driver();

    /**
     * What are we generating?
     */
    enum class CompilationTarget {
        Executable,
        Object,
        Assembly
    };

    [[nodiscard]] CompilationTarget getCompilationTarget() const { return m_compilationTarget; }
    void setCompilationTarget(CompilationTarget target) { m_compilationTarget = target; }

    /**
     * TypeFirst of output. Native asm/obj or
     * LLVM IR/ButCode
     */
    enum class OutputType {
        Native,
        LLVM
    };

    [[nodiscard]] OutputType getOutputType() const { return m_outputType; }
    void setOutputType(OutputType outputType) { m_outputType = outputType; }

    /**
     * Optimization level
     */
    enum class OptimizationLevel {
        O0,
        OS,
        O1,
        O2,
        O3
    };

    [[nodiscard]] static const char* getCmdOption(OptimizationLevel level);
    [[nodiscard]] OptimizationLevel getOptimizationLevel() const { return m_optimizationLevel; }
    void setOptimizationLevel(OptimizationLevel level) { m_optimizationLevel = level; }

    [[nodiscard]] bool isDebugBuild() const { return m_isDebug; }
    void setDebugBuild(bool debug) { m_isDebug = debug; }

    enum class FileType {
        Source,   // anything, but mostly .bas
        Assembly, // .s
        Object,   // .o
        LLVMIr,   // .ll
        BitCode,  // .bc
        count
    };

    [[nodiscard]] static constexpr const char* getFileExt(FileType type) {
        switch (type) {
        case FileType::Source:
            return ".bas";
        case FileType::Assembly:
            return ".s";
        case FileType::Object:
            return ".o";
        case FileType::LLVMIr:
            return ".ll";
        case FileType::BitCode:
            return ".bc";
        default:
            llvm_unreachable("Invalid file type");
        }
    }

    void addInputFile(const fs::path& path);
    [[nodiscard]] const std::vector<fs::path>& getInputFiles(FileType type) const;

    /**
     * Tools that driver can make use of
     */
    enum class Tool {
        Optimizer, // llvm optimizer
        Assembler, // llvm assembler
        Linker,    // linker
        Count
    };

    /**
     * Find path to the given tool
     * @param tool path
     * @return
     */
    [[nodiscard]] static fs::path getToolPath(Tool tool);

    /**
     * Set current working directory which is used
     * for resolving file paths
     */
    void setWorkingDir(const fs::path& path);

    /**
     * Get current working directory
     */
    [[nodiscard]] const fs::path& getWorkingDir() const { return m_workingDir; }

    /**
     * Set path to compiler binary.
     */
    void setCompilerPath(const fs::path& path) { m_compilerPath = path; }

    /**
     * Get compiler path
     */
    [[nodiscard]] const fs::path& getCompilerPath() const { return m_compilerPath; }

    /**
     * Set output file path or a directory
     */
    void setOutputFilePath(const fs::path& path);

    /**
     * Get defined output path or an empty path if none is set
     */
    [[nodiscard]] const fs::path& getOutputPath() const { return m_outputFilePath; }

    /**
     * Set compiler log generation to verbose
     */
    void setVerbose(bool verbose) { m_verbose = verbose; }

    /**
     * Get compiler log generation is verbose
     */
    [[nodiscard]] bool getVerbose() const { return m_verbose; }

    /**
     * Set the triple used for code generation
     */
    void setTriple(const llvm::Triple& triple) { m_triple = triple; }

    /**
     * Get currently defined triple
     */
    [[nodiscard]] llvm::Triple& getTriple() { return m_triple; }

    /**
     * Get source manager
     */
    [[nodiscard]] llvm::SourceMgr& getSourceMrg() { return m_sourceMgr; }

    /**
     * Get LLVM Context
     */
    [[nodiscard]] llvm::LLVMContext& getLlvmContext() { return m_llvmContext; }

    int execute();

private:
    void validate();

    [[nodiscard]] bool isTargetLinkable() const {
        return m_compilationTarget == CompilationTarget::Executable;
    }

    [[nodiscard]] bool isTargetNative() const {
        return m_compilationTarget == CompilationTarget::Executable;
    }

    [[nodiscard]] bool outputLLVMIr() const {
        return m_outputType == OutputType::LLVM && m_compilationTarget == CompilationTarget::Assembly;
    }

    void emitLLVMIr();
    std::vector<fs::path> emitBitCode(bool final);
    std::vector<fs::path> emitNative(CompilationTarget emit, bool final);
    void emitExecutable();

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
     *
     * @param path relative or absolute path
     * @param type path kind
     * @return resolved path
     */
    [[nodiscard]] fs::path resolvePath(const fs::path& path) const;

    /**
     * Validate that file exists and is a valid readable file
     *
     * @param path
     * @param mustExist if file does not exist lbc will quit with an error
     * @return true if file is valid
     */
    [[nodiscard]] static bool validateFile(const fs::path& path, bool mustExist);

    /**
     * Compile all sources
     */
    void compileSources();

    /**
     * Compile given file
     *
     * @param path file path
     * @param ID in SourceManager
     */
    void compileSource(const fs::path& path, unsigned ID);

    bool m_verbose = false;

    std::array<std::vector<fs::path>, static_cast<size_t>(FileType::count)> m_inputFiles{};

    OutputType m_outputType = OutputType::Native;
    CompilationTarget m_compilationTarget = CompilationTarget::Executable;
    OptimizationLevel m_optimizationLevel = OptimizationLevel::O2;
    bool m_isDebug = false;

    fs::path m_workingDir{};
    fs::path m_compilerPath{};
    fs::path m_outputFilePath{};

    llvm::Triple m_triple;
    llvm::SourceMgr m_sourceMgr{};
    llvm::LLVMContext m_llvmContext{};
    std::vector<unique_ptr<llvm::Module>> m_modules{};
};

} // namespace lbc
