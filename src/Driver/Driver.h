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
    enum class CompileResult {
        LLVMIr,
        BitCode,
        Assembly,
        Object,
        Executable,
        Library
    };

    [[nodiscard]] CompileResult getResult() const { return m_result; }
    void setResult(CompileResult result) { m_result = result; }

    /**
     * Optimization level
     */
    enum class OptimizationLevel {
        O0,
        O1,
        O2,
        O3
    };

    [[nodiscard]] static string getCmdOption(OptimizationLevel level);
    [[nodiscard]] OptimizationLevel getLevel() const { return m_level; }
    void setLevel(OptimizationLevel level) { m_level = level; }

    /**
     * Build type
     */
    enum class BuildMode {
        Debug,
        Release
    };

    [[nodiscard]] BuildMode getMode() const { return m_mode; }
    void setMode(BuildMode mode) { m_mode = mode; }

    /**
     * Resource type. Paths, source files, linked libraries, etc.
     */
    enum class ResourceType {
        SourceDirectory,
        Source,
        LibraryDirectory,
        Library,
        Count
    };
    using ResourceContainer = std::set<fs::path>;

    void addResource(ResourceType type, const fs::path& path);
    [[nodiscard]] const ResourceContainer& getResources(ResourceType type) const;

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
    [[nodiscard]] static fs::path getToolPath(Tool tool) ;

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
    void setOutputPath(const fs::path& path);

    /**
     * Get defined output path or an empty path if none is set
     */
    [[nodiscard]] const fs::path& getOutputPath() const { return m_outputPath; }

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
    void emitLLVMIr();
    std::vector<fs::path> emitBitCode(bool final);
    std::vector<fs::path> emitNative(CompileResult emit, bool final);
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
    [[nodiscard]] fs::path resolvePath(const fs::path& path, ResourceType type) const;

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

    /**
     * Optimize given files, in-place
     */
    std::vector<fs::path> optimize(const std::vector<fs::path>&, CompileResult emit, bool final);

    std::array<ResourceContainer, static_cast<size_t>(ResourceType::Count)> m_resources{};

    CompileResult m_result = CompileResult::Executable;
    OptimizationLevel m_level = OptimizationLevel::O2;
    BuildMode m_mode = BuildMode::Debug;

    fs::path m_workingDir{};
    fs::path m_compilerPath{};
    fs::path m_outputPath{};

    bool m_verbose = false;

    llvm::Triple m_triple;
    llvm::SourceMgr m_sourceMgr{};
    llvm::LLVMContext m_llvmContext{};
    std::vector<unique_ptr<llvm::Module>> m_modules{};
};

} // namespace lbc
