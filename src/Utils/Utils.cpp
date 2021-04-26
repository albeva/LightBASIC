//
// Created by Albert Varaksin on 18/04/2021.
//
#include "Utils.h"
#include "Driver/TempFileCache.h"

[[noreturn]] void lbc::fatalError(const string& message) {
    TempFileCache::removeTemporaryFiles();

    std::cerr << "lbc: error: " << message << '\n';
    std::exit(EXIT_FAILURE);
}
