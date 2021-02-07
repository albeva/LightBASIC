//
// Created by Albert on 07/02/2021.
//
#include "pch.h"
#pragma once

namespace lbc {

class Toolchain;
enum class ToolKind;

class ToolTask final {
public:
    explicit ToolTask(ToolKind kind, fs::path  path);

    void addArg(string arg);
    void addPath(const fs::path& path);

    int run();
    void reset();

private:
    std::vector<string> m_args;

    const ToolKind m_kind;
    const fs::path m_path;
};

} // namespace lbc
