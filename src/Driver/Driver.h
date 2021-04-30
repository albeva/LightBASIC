//
// Created by Albert Varaksin on 13/07/2020.
//
#pragma once
#include "pch.h"
#include "Context.h"
#include "Source.h"
#include "Toolchain/Toolchain.h"

namespace lbc {

/**
 * Drive compilation process
 */
class Driver final : private NonCopyable {
public:
    explicit Driver(Context& context) noexcept : m_context{ context } {}
    int drive() noexcept;

private:
    using SourceVector = std::vector<unique_ptr<Source>>;

    void processInputs() noexcept;
    [[nodiscard]] std::unique_ptr<Source> deriveSource(const Source& source, Context::FileType type, bool temporary) const noexcept;
    SourceVector& getSources(Context::FileType type) noexcept {
        return m_sources.at(static_cast<size_t>(type));
    }

    void emitLLVMIr(bool temporary) noexcept;
    void emitBitCode(bool temporary) noexcept;
    void emitLlvm(Context::FileType type, bool temporary, void (*generator)(llvm::raw_fd_ostream&, llvm::Module&)) noexcept;
    void emitAssembly(bool temporary) noexcept;
    void emitObjects(bool temporary) noexcept;
    void emitNative(Context::FileType type, bool temporary) noexcept;
    void emitExecutable() noexcept;

    void compileSources() noexcept;
    void compileSource(const Source* source, unsigned ID) noexcept;

    Context& m_context;
    std::array<SourceVector, Context::fileTypeCount> m_sources;
    std::vector<std::pair<unique_ptr<llvm::Module>, const Source*>> m_modules{};
};

} // namespace lbc
