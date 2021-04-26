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
    void emitLLVMIr(bool temporary);
    void emitBitCode(bool temporary);
    void emitAssembly(bool temporary);
    void emitObjects(bool temporary);
    void emitExecutable();

    void compileSources();
    void compileSource(const fs::path& path, unsigned ID);

    struct Origin final { // NOLINT
        Context::FileType type;
        size_t index;
        Origin(Context::FileType type, size_t index)
        : type{ type }, index{ index } {}
    };

    struct Artefact final { // NOLINT
        Origin origin;
        fs::path path;
        Artefact(Context::FileType type, size_t index, fs::path path)
        : origin{ type, index }, path{ std::move(path) } {}

        Artefact(Origin origin, fs::path path)
        : origin{ origin }, path{ std::move(path) } {}
    };

    std::vector<Artefact>& getArtifacts(Context::FileType type) {
        return m_inputs.at(static_cast<size_t>(type));
    }

    const fs::path& getOrigin(const Artefact& artefact) {
        return getArtifacts(artefact.origin.type)[artefact.origin.index].path;
    }

    Artefact* findArtifact(Context::FileType type, const fs::path&);
    Artefact* findArtifact(const fs::path& path);

    Context& m_context;
    std::vector<unique_ptr<llvm::Module>> m_modules{};
    std::array<std::vector<Artefact>, Context::fileTypeCount> m_inputs;
};

} // namespace lbc
