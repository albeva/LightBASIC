//
// Created by Albert on 07/02/2021.
//
#pragma once

namespace lbc {

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

    enum class Tool {
        Optimizer, // optimizer
        Assembler, // assembler
        Linker,    // linker
    };

    /**
     * Set toolchain base path
     * @param path
     */
    void setBasePath(fs::path path);

    /**
     * Get path for the given tool
     */
    [[nodiscard]] fs::path getPath(Tool tool);
};

} // namespace lbc
