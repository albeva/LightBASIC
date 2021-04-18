//
// Created by Albert on 03/07/2020.
//
#include "pch.h"
#include "Driver/CmdLineParser.h"
#include "Driver/Context.h"
#include "Driver/Driver.h"
using namespace lbc;

int main(int argc, const char* argv[]) {
    Context context;
    CmdLineParser cmdLineParser{ context };
    cmdLineParser.parse({ argv, static_cast<size_t>(argc) });
    context.validate();

    Driver driver{ context };
    return driver.execute();
}
