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
    ToolTask(Context& context, const fs::path& path) noexcept
    : m_context{ context }, m_path{ path } {}

    ToolTask& reset() noexcept;

    ToolTask& addArg(string arg) noexcept;
    ToolTask& addArg(string name, string value) noexcept;
    ToolTask& addPath(const fs::path& path) noexcept;
    ToolTask& addPath(string name, const fs::path& value) noexcept;
    ToolTask& addArgs(std::initializer_list<string> arghs) noexcept;

    [[nodiscard]] int execute() const noexcept;

private:
    std::vector<string> m_args;
    Context& m_context;
    const fs::path m_path;
};

} // namespace lbc
