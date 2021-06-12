//
// Created by Albert Varaksin on 13/07/2020.
//
#pragma once
#include "Ast/Ast.hpp"
#include "CompileOptions.hpp"
#include "Source.hpp"
#include "TranslationUnit.hpp"

namespace lbc {
class Context;

/**
 * Drive compilation process
 */
class Driver final {
public:
    NO_COPY_AND_MOVE(Driver)

    explicit Driver(Context& context) noexcept;
    ~Driver() noexcept = default;

    void drive();

private:
    using SourceVector = std::vector<unique_ptr<Source>>;

    void processInputs();
    [[nodiscard]] std::unique_ptr<Source> deriveSource(const Source& source, CompileOptions::FileType type, bool temporary) const noexcept;
    [[nodiscard]] SourceVector& getSources(CompileOptions::FileType type) {
        return m_sources.at(static_cast<size_t>(type));
    }

    void emitLLVMIr(bool temporary);
    void emitBitCode(bool temporary);
    void emitLlvm(CompileOptions::FileType type, bool temporary, void (*generator)(llvm::raw_fd_ostream&, llvm::Module&));
    void emitAssembly(bool temporary);
    void emitObjects(bool temporary);
    void emitNative(CompileOptions::FileType type, bool temporary);
    void emitExecutable();

    void optimize();

    void compileSources();
    void compileSource(const Source* source, unsigned ID);
    void dumpAst();

    Context& m_context;
    const CompileOptions& m_options;

    std::array<SourceVector, CompileOptions::FILETYPE_COUNT> m_sources{};
    std::vector<unique_ptr<TranslationUnit>> m_modules{};
    void dumpCode();
};

} // namespace lbc
