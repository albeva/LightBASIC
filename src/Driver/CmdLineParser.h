//
// Created by Albert Varaksin on 17/04/2021.
//
#pragma once
#include "pch.h"

namespace lbc {

class Context;

/**
 * Parse given commandline arguments
 * and build up the context
 */
class CmdLineParser final {
    NON_COPYABLE(CmdLineParser)
public:
    using Args = llvm::ArrayRef<const char*>;

    explicit CmdLineParser(Context& context) noexcept;
    ~CmdLineParser();

    void parse(const Args& args) noexcept;

private:
    void processOption(const Args& args, size_t& index) noexcept;

    [[noreturn]] static void showError(const string& message) noexcept;
    [[noreturn]] static void showHelp() noexcept;
    [[noreturn]] static void showVersion() noexcept;

    Context& m_context;
};

} // namespace lbc
