//
// Created by Albert on 15/07/2020.
//
#pragma once
#include <utility>

#include "pch.h"

namespace lbc {

class Tool {
    NON_COPYABLE(Tool)
public:
    virtual ~Tool() = default;

    virtual void execute(const llvm::ArrayRef<fs::path>& input, fs::path output) = 0;
};

} // namespace lbc
