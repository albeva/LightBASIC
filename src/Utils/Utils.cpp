//
// Created by Albert Varaksin on 18/04/2021.
//
#include "Utils.hpp"
#include "Driver/TempFileCache.hpp"

void lbc::fatalError(const Twine& message, bool prefix) noexcept {
    TempFileCache::removeTemporaryFiles();

    if (prefix) {
        std::cerr << "lbc: error: ";
    }
    std::cerr << message.str() << std::endl;

    std::exit(EXIT_FAILURE);
}

void lbc::warning(const Twine& message, bool prefix) noexcept {
    if (prefix) {
        std::cout << "lbc: warning: ";
    }
    std::cout << message.str() << std::endl;
}
