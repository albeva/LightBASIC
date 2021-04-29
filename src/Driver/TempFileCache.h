//
//  TempFileCache.hpp
//  lbc
//
//  Created by Albert Varaksin on 26/04/2021.
//
#pragma once
#include "pch.h"

namespace lbc::TempFileCache {
[[nodiscard]] fs::path createUniquePath(const StringRef& suffix);
[[nodiscard]] fs::path createUniquePath(const fs::path& file, const StringRef& suffix);
void removeTemporaryFiles();
} // namespace lbc::TempFileCache
