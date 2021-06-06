//
// Created by Albert on 06/06/2021.
//
#pragma once

namespace lbc {
class Symbol;

struct ValueCategory final {
    enum Flags {
        None = 0,
        Addressable = 1,
        Dereferencable = 2,
        Assignable = 4,
        Callable = 8,
        LLVM_MARK_AS_BITMASK_ENUM(Callable)
    };

    constexpr ValueCategory() noexcept = default;
    constexpr explicit ValueCategory(Flags flags) : m_flags{ flags } {}

    constexpr ValueCategory& operator=(Flags flags) noexcept {
        m_flags = flags;
        return *this;
    }

    [[nodiscard]] Flags constexpr getFlags() const noexcept { return m_flags; }
    void constexpr setFlags(Flags flags) noexcept { m_flags = flags; }

    void inline set(Flags flags) noexcept { m_flags |= flags; }
    void inline unset(Flags flags) noexcept { m_flags &= ~flags; }

    [[nodiscard]] constexpr bool canAddress() const noexcept {
        return (m_flags & Addressable) != 0;
    }

    [[nodiscard]] constexpr bool canDereference() const noexcept {
        return (m_flags & Dereferencable) != 0;
    }

    [[nodiscard]] constexpr bool canAssign() const noexcept {
        return (m_flags & Assignable) != 0;
    }

    [[nodiscard]] constexpr bool canCall() const noexcept {
        return (m_flags & Callable) != 0;
    }

private:
    Flags m_flags = None;
};

} // namespace lbc
