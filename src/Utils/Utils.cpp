//
// Created by Albert Varaksin on 18/04/2021.
//
#include "Utils.h"
#include "Driver/TempFileCache.h"

void lbc::fatalError(const Twine& message, bool prefix) {
    TempFileCache::removeTemporaryFiles();

    if (prefix) {
        std::cerr << "lbc: error: ";
    }
    std::cerr << message.str() << std::endl;

    std::exit(EXIT_FAILURE);
}
