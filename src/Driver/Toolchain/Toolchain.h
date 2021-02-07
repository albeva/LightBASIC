//
// Created by Albert on 07/02/2021.
//
#include "pch.h"
#pragma once

namespace lbc {

class ToolTask;

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
class Toolchain final {
    NON_COPYABLE(Toolchain)
public:
    ~Toolchain() = default;

    /**
     * Set toolchain base path
     * @param path
     */
    void setBasePath(fs::path path);

    /**
     * Get path for the given tool
     */
    [[nodiscard]] fs::path getPath(ToolKind tool);

    ToolTask createTask(ToolKind kind);
};

} // namespace lbc
