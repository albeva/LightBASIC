//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
// do not include pch.h!

#define LOG_VAR(VAR) std::cout << #VAR << " = " << VAR << '\n';

#ifdef __PRETTY_FUNCTION__
#    define LBC_FUNCTION __PRETTY_FUNCTION__
#else
#    define LBC_FUNCTION __FUNCTION__
#endif

namespace lbc {

class NonCopyable {
public:
    constexpr NonCopyable() = default;
    ~NonCopyable() = default;

    NonCopyable(NonCopyable&&) = delete;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(NonCopyable&&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

/**
 * End compilation error with the message, clear the state and exit with error
 *
 * @param message to print
 * @param prefix add standard prefix befor ethe message
 */
[[noreturn]] void fatalError(const string& message, bool prefix = true);

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
struct ValueRestorer {
public:
    explicit ValueRestorer(T& value) : m_target{ value }, m_value{ value } {}

    // restore
    ~ValueRestorer() {
        m_target = m_value;
    }

    ValueRestorer(ValueRestorer&&) = delete;
    ValueRestorer(const ValueRestorer&) = delete;
    ValueRestorer& operator=(ValueRestorer&&) = delete;
    ValueRestorer& operator=(const ValueRestorer&) = delete;

    // members
private:
    T& m_target;
    T m_value;
};

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define MAKE_UNIQUE(x) CONCATENATE(x, __COUNTER__)
#define RESTORE_ON_EXIT(V) ValueRestorer<decltype(V)> MAKE_UNIQUE(tmp_restore_onexit_){ V };

} // namespace lbc
