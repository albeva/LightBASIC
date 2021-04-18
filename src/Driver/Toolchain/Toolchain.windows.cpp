//
// Created by Albert on 07/02/2021.
//
#include "Toolchain.h"

fs::path Toolchain::getPath(ToolKind tool) {
    fs::path path;
    switch (tool) {
    case ToolKind::Optimizer:
        path = "c:/dev/bin/opt.exe";
        break;
    case ToolKind::Assembler:
        path = "c:/dev/bin/llc.exe";
        break;
    case ToolKind::Linker:
        path = "c:/dev/bin/ld.exe";
        break;
    default:
        llvm_unreachable("Invalid ToolKind ID");
    }
    if (!fs::exists(path)) {
        fatalError("ToolKind "s + path.string() + " not found!");
    }
    return path;
}
