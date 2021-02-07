//
// Created by Albert on 07/02/2021.
//
#include "Toolchain.h"

#if __APPLE__ || __linux__ || __unix__
    #include "Toolchain.unix.cpp"
#else
    #error "Unsupported platform"
#endif


