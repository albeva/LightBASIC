//
// Created by Albert on 07/02/2021.
//
#include "Toolchain.h"

using namespace lbc;

[[noreturn]] static void error(const string& message) {
    std::cerr << "lbc: error: " << message << '\n';
    std::exit(EXIT_FAILURE);
}

void Toolchain::setBasePath(fs::path /*path*/) {
    // TODO
    error("Setting toolchain basepath is not implemented");
}

fs::path Toolchain::getPath(Toolchain::Tool tool) {
    fs::path path;
    switch (tool) {
    case Tool::Optimizer:
        path = "/usr/local/bin/opt";
        break;
    case Tool::Assembler:
        path = "/usr/local/bin/llc";
        break;
    case Tool::Linker:
        path = "/usr/bin/ld";
        break;
    default:
        llvm_unreachable("Invalid Tool ID");
    }
    if (!fs::exists(path)) {
        error("Tool "s + path.string() + " not found!");
    }
    return path;
}
