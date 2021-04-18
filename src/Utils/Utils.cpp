//
// Created by Albert on 18/04/2021.
//
#include "pch.h"

[[noreturn]] void lbc::fatalError(const string& message) {
    std::cerr << "lbc: error: " << message << '\n';
    std::exit(EXIT_FAILURE);
}