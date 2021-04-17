//
// Created by Albert on 03/07/2020.
//
#include "pch.h"
#include "Driver/Driver.h"
#include "Driver/CmdLineParser.h"
#include <llvm/Support/FileSystem.h>

using namespace lbc;

int main(int argc, const char* argv[]) {
    Driver driver;

    CmdLineParser cmdLineParser{driver};
    cmdLineParser.parse({argv, static_cast<size_t>(argc)});

    return driver.execute();
}
