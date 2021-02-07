//
// Created by Albert on 07/02/2021.
//
#include "ToolTask.h"

#include <utility>

using namespace lbc;

ToolTask::ToolTask(ToolKind kind, fs::path path)
    : m_kind{kind}
    , m_path{std::move(path)} {
}

int ToolTask::run() {
    std::vector<llvm::StringRef> args;
    args.reserve(m_args.size());
    for (const auto& arg: m_args) {
        args.emplace_back(arg);
    }

    return llvm::sys::ExecuteAndWait(m_path.string(), args);
}

void ToolTask::addArg(string arg) {
    m_args.push_back(std::move(arg));
}

void ToolTask::addPath(const fs::path& path) {
    addArg(path.string());
}

void ToolTask::reset() {
    m_args.clear();
}
