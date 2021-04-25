//
// Created by Albert Varaksin on 07/02/2021.
//
#include "Toolchain.h"

fs::path Toolchain::getPath(ToolKind tool) {
    fs::path path;
    switch (tool) {
    case ToolKind::Optimizer:
        path = m_basePath / "opt";
        break;
    case ToolKind::Assembler:
        path = m_basePath / "llc";
        break;
    case ToolKind::Linker:
        path = m_basePath / "ld";
        break;
    default:
        llvm_unreachable("Invalid ToolKind ID");
    }
    if (!fs::exists(path)) {
        fatalError("ToolKind "s + path.string() + " not found!");
    }
    return path;
}
