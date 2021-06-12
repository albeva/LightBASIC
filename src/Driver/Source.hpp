//
// Created by Albert Varaksin on 30/04/2021.
//
#pragma once
#include "CompileOptions.hpp"
#include <utility>

namespace lbc {

/**
 * Class that represents a source file
 */
struct Source final {
    NO_COPY_AND_MOVE(Source)

    Source(CompileOptions::FileType ty, fs::path p, bool gen, const Source* o) noexcept
    : type{ ty }, path{ std::move(p) }, isGenerated{ gen }, origin{ o == nullptr ? *this : *o } {}

    ~Source() noexcept = default;

    [[nodiscard]] static unique_ptr<Source> create(CompileOptions::FileType type, fs::path path, bool generated, const Source* origin = nullptr) {
        return make_unique<Source>(type, std::forward<fs::path>(path), generated, origin);
    }

    const CompileOptions::FileType type;
    const fs::path path;
    const bool isGenerated;
    const Source& origin;

    /**
     * Derive new generated Source with the same origin
     */
    [[nodiscard]] unique_ptr<Source> derive(CompileOptions::FileType ty, fs::path p) const {
        return create(ty, std::move(p), true, &origin);
    }
};

} // namespace lbc