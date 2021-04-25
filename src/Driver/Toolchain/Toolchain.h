//
// Created by Albert Varaksin on 07/02/2021.
//
#pragma once
#include "pch.h"

namespace lbc {

class ToolTask;
class Context;

enum class ToolKind {
    Optimizer, // optimizer
    Assembler, // assembler
    Linker,    // linker
};

/**
 * Abstract access and execution of tools
 * used during compilation
 *
 * e.g. a linker
 */
class Toolchain final : private NonCopyable {
public:
    explicit Toolchain(Context& context) : m_context{ context } {}

    /**
     * Set toolchain base path
     * @param path to llvm toolchain
     */
    void setBasePath(fs::path path) { m_basePath = path; }
    const fs::path& getBasePath() const { return m_basePath; }

    /**
     * Get path for the given tool
     */
    [[nodiscard]] fs::path getPath(ToolKind tool);

    ToolTask createTask(ToolKind kind);

private:
    fs::path m_basePath{};
    Context& m_context;
};

} // namespace lbc
