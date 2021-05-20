//
// Created by Albert Varaksin on 20/05/2021.
//
#pragma once
// do not include pch.h!

namespace lbc {

template<typename E, std::enable_if_t<std::is_enum_v<E>, int> = 0>
struct EnableBitMaskOperators {
    constexpr static bool enable = false;
};

#define ENABLE_BITMASK_OPERATORS(E)          \
    template<>                               \
    struct EnableBitMaskOperators<E> {       \
        constexpr static bool enable = true; \
    };

// ~flag

template<typename T, std::enable_if_t<EnableBitMaskOperators<T>::enable, int> = 0>
constexpr T operator~(const T lhs) noexcept {
    using B = std::underlying_type_t<T>;
    return static_cast<T>(~static_cast<B>(lhs));
}

// f1 | f2

template<typename T, std::enable_if_t<EnableBitMaskOperators<T>::enable, int> = 0>
constexpr T operator|(const T lhs, const T rhs) noexcept {
    using B = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<B>(lhs) | static_cast<B>(rhs));
}

// f1 & f2

template<typename T, std::enable_if_t<EnableBitMaskOperators<T>::enable, int> = 0>
constexpr T operator&(const T lhs, const T rhs) noexcept {
    using B = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<B>(lhs) & static_cast<B>(rhs));
}

// f1 ^ f2

template<typename T, std::enable_if_t<EnableBitMaskOperators<T>::enable, int> = 0>
constexpr T operator^(const T lhs, const T rhs) noexcept {
    using B = std::underlying_type_t<T>;
    return static_cast<T>(static_cast<B>(lhs) ^ static_cast<B>(rhs));
}

// f1 |= f2

template<typename T, std::enable_if_t<EnableBitMaskOperators<T>::enable, int> = 0>
constexpr T& operator|=(T& lhs, const T rhs) noexcept {
    return lhs = lhs | rhs;
}

// f1 &= f2

template<typename T, std::enable_if_t<EnableBitMaskOperators<T>::enable, int> = 0>
constexpr T& operator&=(T& lhs, const T rhs) noexcept {
    return lhs = lhs & rhs;
}

// f1 ^= f2

template<typename T, std::enable_if_t<EnableBitMaskOperators<T>::enable, int> = 0>
constexpr T& operator^=(T& lhs, const T rhs) noexcept {
    return lhs = lhs ^ rhs;
}

// f == 0

template<typename T, typename B = std::underlying_type_t<T>, std::enable_if_t<EnableBitMaskOperators<T>::enable, int> = 0>
constexpr bool operator==(const T lhs, const B rhs) noexcept {
    return static_cast<B>(lhs) == rhs;
}

// f != 0

template<typename T, typename B = std::underlying_type_t<T>, std::enable_if_t<EnableBitMaskOperators<T>::enable, int> = 0>
constexpr bool operator!=(const T lhs, const B rhs) noexcept {
    return static_cast<B>(lhs) != rhs;
}

} // namespace lbc
