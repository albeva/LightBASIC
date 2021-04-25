//
// Created by Albert Varaksin on 07/02/2021.
//
#pragma once
#include "pch.h"

namespace lbc {

class Toolchain;
class Context;
enum class ToolKind;

class ToolTask final : private NonCopyable {
public:
    ToolTask(Context& context, ToolKind kind, fs::path path);
    ToolTask& reserve(size_t newCap);

    ToolTask& reset();
    ToolTask& addArg(string arg);
    ToolTask& addArg(string name, string value);
    ToolTask& addPath(const fs::path& path);
    ToolTask& addPath(string name, const fs::path& value);
    ToolTask& addArgs(std::initializer_list<string> arghs);
    int execute();

private:
    std::vector<string> m_args;
    Context& m_context;
    const ToolKind m_kind;
    const fs::path m_path;
};

} // namespace lbc
