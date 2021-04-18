//
// Created by Albert on 07/02/2021.
//
#include "Toolchain.h"
#include "ToolTask.h"

using namespace lbc;

void Toolchain::setBasePath(fs::path /*path*/) {
    // TODO
    fatalError("Setting toolchain basepath is not implemented");
}

ToolTask Toolchain::createTask(ToolKind kind) {
    return ToolTask(kind, getPath(kind));
}

#if __APPLE__ || __linux__ || __unix__
#    include "Toolchain.unix.cpp"
#else
#    include "Toolchain.windows.cpp"
#endif