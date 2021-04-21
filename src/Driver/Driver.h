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
class Driver final: private NonCopyable {
public:
    explicit Driver(Context& m_context);

    int execute();
    [[nodiscard]] Context& getContext() { return m_context; }

private:
    void emitLLVMIr();
    [[nodiscard]] std::vector<fs::path> emitBitCode(bool final);
    [[nodiscard]] std::vector<fs::path> emitNative(Context::CompilationTarget emit, bool final);
    void emitExecutable();

    void compileSources();
    void compileSource(const fs::path& path, unsigned ID);

    Context& m_context;
    std::vector<unique_ptr<llvm::Module>> m_modules{};
};

} // namespace lbc
