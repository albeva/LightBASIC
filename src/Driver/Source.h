//
// Created by Albert on 30/04/2021.
//
#pragma once
#include "pch.h"
#include "Context.h"

namespace lbc {

/**
 * Class that represents a source file
 */
struct Source final {
    NO_COPY_AND_MOVE(Source)

    Source(Context::FileType ty, const fs::path& p, bool gen, const Source* o) noexcept
    : type{ ty }, path{ p }, isGenerated{ gen }, origin{ o == nullptr ? *this : *o } {}

    ~Source() = default;

    [[nodiscard]] static unique_ptr<Source> create(Context::FileType type, const fs::path& path, bool generated, const Source* origin = nullptr) noexcept {
        return make_unique<Source>(type, path, generated, origin);
    }

    const Context::FileType type;
    const fs::path path;
    const bool isGenerated;
    const Source& origin;

    /**
     * Derive new generated Source with the same origin
     */
    [[nodiscard]] unique_ptr<Source> derive(Context::FileType ty, const fs::path& p) const noexcept {
        return create(ty, p, true, &origin);
    }
};

} // namespace lbc