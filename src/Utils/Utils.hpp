//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
// do not include pch.h!

#define LOG_VAR(VAR) llvm::outs() << #VAR << " = " << VAR << '\n';

#define NO_COPY_AND_MOVE(Class)         \
    Class(Class&&) = delete;            \
    Class(const Class&) = delete;       \
    Class& operator=(Class&&) = delete; \
    Class& operator=(const Class&) = delete;

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define MAKE_UNIQUE(x) CONCATENATE(x, __COUNTER__)

namespace lbc {
// Enum Flags
LLVM_ENABLE_BITMASK_ENUMS_IN_NAMESPACE();

template<typename E, typename = std::enable_if_t<llvm::is_bitmask_enum<E>::value>>
constexpr bool operator==(E lhs, std::underlying_type_t<E> rhs) noexcept {
    return lhs == static_cast<E>(rhs);
}

template<typename E, typename = std::enable_if_t<llvm::is_bitmask_enum<E>::value>>
constexpr bool operator!=(E lhs, std::underlying_type_t<E> rhs) noexcept {
    return lhs != static_cast<E>(rhs);
}

// helper type for std::variant visitors
template<typename... Base>
struct Visitor : Base... { using Base::operator()...; };

template<typename... Base>
Visitor(Base...) -> Visitor<Base...>;

/**
 * Get Twine from "literal"_t
 */
inline Twine operator"" _t(const char* s, size_t /*len*/) noexcept {
    return { s };
}

/**
 * End compilation with the error message, clear the state and exit with error
 *
 * @param message to print
 * @param prefix add standard prefix before the message
 */
[[noreturn]] void fatalError(const Twine& message, bool prefix = true);

/**
 * Emit compiler warning, but continue compilation process
 * @param message to print
 * @param prefix add standard prefix before the message
 */
void warning(const Twine& message, bool prefix = true);

/**
 * Helper class that restores variable value when existing scope
 *
 * Example usage:
 *
 *    int foo = 5;
 *    {
 *        RESTORE_ON_EXIT(foo); // use macro defined below.
 *        foo = 10;
 *        assert(foo == 10);
 *    }
 *    assert(foo == 5);
 */
template<typename T, std::enable_if_t<std::is_trivially_copyable_v<T> && std::is_trivially_assignable_v<T&, T>, int> = 0>
struct ValueRestorer final {
    NO_COPY_AND_MOVE(ValueRestorer)

    constexpr explicit ValueRestorer(T& value) noexcept : m_target{ value }, m_value{ value } {}

    ~ValueRestorer() noexcept {
        m_target = m_value;
    }

private:
    T& m_target;
    const T m_value;
};

#define RESTORE_ON_EXIT(V)                                            /* NOLINT */ \
    ValueRestorer<decltype(V)> MAKE_UNIQUE(tmp_restore_onexit_) { V } /* NOLINT */

} // namespace lbc
