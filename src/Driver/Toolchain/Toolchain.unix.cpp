//
// Created by Albert on 07/02/2021.
//
#include "Toolchain.h"

fs::path Toolchain::getPath(ToolKind tool) {
    fs::path path;
    switch (tool) {
    case ToolKind::Optimizer:
        path = "/usr/local/bin/opt";
        break;
    case ToolKind::Assembler:
        path = "/usr/local/bin/llc";
        break;
    case ToolKind::Linker:
        path = "/usr/bin/ld";
        break;
    default:
        llvm_unreachable("Invalid ToolKind ID");
    }
    if (!fs::exists(path)) {
        error("ToolKind "s + path.string() + " not found!");
    }
    return path;
}
