//
// Created by Albert Varaksin on 03/07/2020.
//
#include "Driver/CmdLineParser.hpp"
#include "Driver/Context.hpp"
#include "Driver/Driver.hpp"
#include <llvm/Support/InitLLVM.h>
using namespace lbc;

int main(int argc, const char* argv[]) {
    llvm::InitLLVM init{ argc, argv };

    Context context{};
    CmdLineParser cmdLineParser{ context };
    cmdLineParser.parse({ argv, static_cast<size_t>(argc) });
    context.validate();

    Driver{ context }.drive();
    return EXIT_SUCCESS;
}
