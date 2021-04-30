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
    explicit Driver(Context& m_context);

    int execute();
    [[nodiscard]] Context& getContext() { return m_context; }

private:
    using SourceContainer = std::vector<unique_ptr<Source>>;

    void processInputs();
    [[nodiscard]] std::unique_ptr<Source> deriveSource(const Source& source, Context::FileType type, bool temporary) const noexcept;

    void emitLLVMIr(bool temporary);
    void emitBitCode(bool temporary);
    void emitAssembly(bool temporary);
    void emitObjects(bool temporary);
    void emitExecutable();

    void compileSources();
    void compileSource(const Source* source, unsigned ID);

    SourceContainer& getSources(Context::FileType type) {
        return m_sources.at(static_cast<size_t>(type));
    }

    Context& m_context;
    std::array<SourceContainer, Context::fileTypeCount> m_sources;
    std::vector<std::pair<unique_ptr<llvm::Module>, const Source*>> m_modules{};
};

} // namespace lbc
