//
// Created by Albert Varaksin on 07/02/2021.
//
#include "Toolchain.h"
#include "ToolTask.h"

using namespace lbc;

ToolTask Toolchain::createTask(ToolKind kind) {
    return ToolTask(m_context, kind, getPath(kind));
}

#if __APPLE__ || __linux__ || __unix__
#    include "Toolchain.unix.cpp"
#else
#    include "Toolchain.windows.cpp"
#endif
