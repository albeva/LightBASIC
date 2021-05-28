//
//  TempFileCache.hpp
//  lbc
//
//  Created by Albert Varaksin on 26/04/2021.
//
#pragma once
#include "pch.hpp"

namespace lbc::TempFileCache {
[[nodiscard]] fs::path createUniquePath(StringRef suffix);
[[nodiscard]] fs::path createUniquePath(const fs::path& file, StringRef suffix);
void removeTemporaryFiles();
} // namespace lbc::TempFileCache
