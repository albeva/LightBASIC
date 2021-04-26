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
        Context::FileType m_type;
        size_t m_index;
        Origin(Context::FileType type, size_t index)
        : m_type{ type }, m_index{ index } {}
    };

    struct Artefact final { // NOLINT
        Origin m_origin;
        fs::path m_path;
        Artefact(Context::FileType type, size_t index, fs::path path)
        : m_origin{ type, index }, m_path{ std::move(path) } {}

        Artefact(Origin origin, fs::path path)
        : m_origin{ origin }, m_path{ std::move(path) } {}
    };

    std::vector<Artefact>& getArtifacts(Context::FileType type) {
        return m_inputs.at(static_cast<size_t>(type));
    }

    const fs::path& getOrigin(const Artefact& artefact) {
        return getArtifacts(artefact.m_origin.m_type)[artefact.m_origin.m_index].m_path;
    }

    Artefact* findArtifact(Context::FileType type, const fs::path&);
    Artefact* findArtifact(const fs::path& path);

    Context& m_context;
    std::vector<unique_ptr<llvm::Module>> m_modules{};
    std::array<std::vector<Artefact>, Context::fileTypeCount> m_inputs;
};

} // namespace lbc
