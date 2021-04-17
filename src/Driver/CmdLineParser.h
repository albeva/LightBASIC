//
// Created by Albert on 17/04/2021.
//

#pragma once
#include "pch.h"

namespace lbc {

class Driver;

class CmdLineParser final {
    NON_COPYABLE(CmdLineParser)
public:
    using Args = llvm::ArrayRef<const char*>;

    explicit CmdLineParser(Driver& driver) noexcept;
    ~CmdLineParser();

    void parse(const Args& args) noexcept;

    [[noreturn]] static void showError(const string& message) noexcept;
private:
    void processOption(const Args& args, size_t& index) noexcept;

    [[noreturn]] static void showHelp() noexcept;
    [[noreturn]] static void showVersion() noexcept;

    Driver& m_driver;
};

} // namespace lbc