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
        None,
        Assembly,
        LLVMIr,
        Object,
        Executable,
        Library
    };

    [[nodiscard]] static string getOptionString(CompileResult result);
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

    [[nodiscard]] static string getOptionString(OptimizationLevel level);
    [[nodiscard]] OptimizationLevel getLevel() const { return m_level; }
    void setLevel(OptimizationLevel level) { m_level = level; }

    /**
     * Build type
     */
    enum class BuildMode {
        Debug,
        Release
    };

    [[nodiscard]] static string getOptionString(BuildMode mode);
    [[nodiscard]] BuildMode getMode() const { return m_mode; }
    void setMode(BuildMode mode) { m_mode = mode; }

    /**
     * Resource type. Paths, source files, linked libraries, etc.
     */
    enum class ResourceType {
        GlobalPath,
        SourcePath,
        SourceFile,
        LibraryPath,
        LibraryFile,
        Count
    };
    using ResourceContainer = std::set<fs::path>;

    void addResource(ResourceType type, const fs::path& path);
    [[nodiscard]] const ResourceContainer& getResources(ResourceType type) const;

    /**
     * Tools that driver can make use of
     */
    enum class Tool {
        Opt, // llvm optimizer
        Llc, // llvm assembler
        Ld,  // linker
        Count
    };

    void setTool(Tool tool, const fs::path& path);
    [[nodiscard]] const fs::path& getTool(Tool tool) const;

    void setWorkingPath(const fs::path& path) { m_workingPath = path; }
    [[nodiscard]] const fs::path& getWorkingPath() const { return m_workingPath; }

    void setCompilerPath(const fs::path& path) { m_compilerPath = path; }
    [[nodiscard]] const fs::path& getCompilerPath() const { return m_compilerPath; }

    void setOutputPath(const fs::path& path) { m_outputPath = path; }
    [[nodiscard]] const fs::path& getOutputPath() const { return m_outputPath; }

    void setVerbose(bool verbose) { m_verbose = verbose; }
    [[nodiscard]] bool getVerbose() const { return m_verbose; }

    void setTriple(const llvm::Triple& triple) { m_triple = triple; }
    [[nodiscard]] llvm::Triple& getTriple() { return m_triple; }

    [[nodiscard]] llvm::SourceMgr& getSourceMrg() { return m_sourceMgr; }

    [[nodiscard]] llvm::LLVMContext& getLlvmContext() { return m_llvmContext; }

    [[nodiscard]] string getOptionsString() const;

private:
    std::array<ResourceContainer, static_cast<size_t>(ResourceType::Count)> m_resources{};
    std::array<fs::path, static_cast<size_t>(Tool::Count)> m_tools{};

    CompileResult m_result = CompileResult::None;
    OptimizationLevel m_level = OptimizationLevel::O0;
    BuildMode m_mode = BuildMode::Debug;

    fs::path m_workingPath{};
    fs::path m_compilerPath{};
    fs::path m_outputPath{};

    bool m_verbose = false;

    llvm::Triple m_triple{};
    llvm::SourceMgr m_sourceMgr{};
    llvm::LLVMContext m_llvmContext{};
};

} // namespace lbc
