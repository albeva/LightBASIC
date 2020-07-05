//
// Created by Albert on 03/07/2020.
//
#pragma once

#define LOG_VAR(VAR) std::cout << #VAR << " = " << VAR << '\n';

namespace lbc {

inline llvm::StringRef view_to_stringRef(const string_view& view) {
    return llvm::StringRef(view.data(), view.size());
}

}