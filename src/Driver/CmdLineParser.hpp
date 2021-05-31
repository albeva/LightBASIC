//
// Created by Albert Varaksin on 17/04/2021.
//
#pragma once
#include "pch.hpp"

namespace lbc {
class Context;

/**
 * Parse given commandline arguments
 * and build up the context
 */
class CmdLineParser final {
public:
    NO_COPY_AND_MOVE(CmdLineParser)

    using Args = llvm::ArrayRef<const char*>;

    explicit CmdLineParser(Context& context) noexcept : m_context{ context } {}
    ~CmdLineParser() noexcept = default;

    void parse(const Args& args);

private:
    void processOption(const Args& args, size_t& index);
    void processToolchainPath(const fs::path& path);

    [[noreturn]] static void showError(const string& message);
    [[noreturn]] static void showHelp();
    [[noreturn]] static void showVersion();

    Context& m_context;
};

} // namespace lbc
