//
// Created by Albert Varaksin on 07/02/2021.
//
#include "ToolTask.h"
#include "Driver/Context.h"

#include <utility>

using namespace lbc;

ToolTask& ToolTask::reset() noexcept {
    m_args.clear();
    return *this;
}

ToolTask& ToolTask::addArg(const string& arg) noexcept {
    m_args.push_back(arg);
    return *this;
}

ToolTask& ToolTask::addArg(const string& name, const string& value) noexcept {
    m_args.push_back(name);
    m_args.push_back(value);
    return *this;
}

ToolTask& ToolTask::addPath(const fs::path& path) noexcept {
    addArg(path.string());
    return *this;
}

ToolTask& ToolTask::addPath(const string& name, const fs::path& value) noexcept {
    m_args.push_back(name);
    addPath(value);
    return *this;
}

ToolTask& ToolTask::addArgs(std::initializer_list<string> args) noexcept {
    if (m_args.capacity() < m_args.size() + args.size()) {
        m_args.reserve(m_args.size() + args.size());
    }
    std::move(args.begin(), args.end(), std::back_inserter(m_args));
    return *this;
}

int ToolTask::execute() const noexcept {
    std::vector<StringRef> args;
    args.reserve(m_args.size() + 1);

    auto program = m_path.string();
    args.emplace_back(program);

    for (const auto& arg : m_args) {
        args.emplace_back(arg);
    }

    if (m_context.isVerbose()) {
        switch (m_kind) {
        case ToolKind::Optimizer:
            std::cout << "Optimize:\n";
            break;
        case ToolKind::Assembler:
            std::cout << "Assemble:\n";
            break;
        case ToolKind::Linker:
            std::cout << "Link:\n";
            break;
        }
        std::cout << program;
        for (const auto& arg : m_args) {
            std::cout << " " << arg;
        }
        std::cout << '\n'
                  << '\n';
    }

    return llvm::sys::ExecuteAndWait(program, args);
}
