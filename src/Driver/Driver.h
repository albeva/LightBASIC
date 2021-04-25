//
// Created by Albert Varaksin on 13/07/2020.
//
#pragma once
#include "pch.h"
#include "Context.h"
#include "Toolchain/Toolchain.h"

namespace lbc {

/**
 * Drive compilation process
 */
class Driver final : private NonCopyable {
public:
    explicit Driver(Context& m_context);

    int execute();
    [[nodiscard]] Context& getContext() { return m_context; }

private:
    void processInputs();
    void emitLLVMIr();
    void emitBitCode();
    void emitObjects();
    void emitExecutable();

    void compileSources();
    void compileSource(const fs::path& path, unsigned ID);

    std::vector<fs::path>& getInputs(Context::FileType type) {
        return m_inputs.at(static_cast<size_t>(type));
    }

    Context& m_context;
    std::vector<unique_ptr<llvm::Module>> m_modules{};
    std::array<std::vector<fs::path>, Context::fileTypeCount> m_inputs;


    //    std::vector<fs::path> m_srcFiles{};
    //    std::vector<fs::path> m_bcFiles{};
    //    std::vector<fs::path> m_llFiles{};
    //    std::vector<fs::path> m_asmFiles{};
    //    std::vector<fs::path> m_objFiles{};
};

} // namespace lbc
