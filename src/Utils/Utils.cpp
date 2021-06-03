//
// Created by Albert Varaksin on 18/04/2021.
//
#include "Utils.hpp"
#include "Driver/TempFileCache.hpp"

void lbc::fatalError(const Twine& message, bool prefix) {
    TempFileCache::removeTemporaryFiles();

    if (prefix) {
        llvm::errs() << "lbc: error: ";
    }
    llvm::errs() << message << '\n';

    std::exit(EXIT_FAILURE);
}

void lbc::warning(const Twine& message, bool prefix) {
    if (prefix) {
        llvm::outs() << "lbc: warning: ";
    }
    llvm::outs() << message << '\n';
}
