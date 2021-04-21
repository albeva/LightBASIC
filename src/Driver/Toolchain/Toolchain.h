//
// Created by Albert Varaksin on 07/02/2021.
//
#pragma once
#include "pch.h"

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
class Toolchain final: private NonCopyable {
public:
    /**
     * Set toolchain base path
     * @param path to llvm toolchain
     */
    void setBasePath(fs::path path);

    /**
     * Get path for the given tool
     */
    [[nodiscard]] fs::path getPath(ToolKind tool);

    ToolTask createTask(ToolKind kind);
};

} // namespace lbc
