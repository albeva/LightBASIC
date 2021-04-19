//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
// do not include pch.h!

#define LOG_VAR(VAR) std::cout << #VAR << " = " << VAR << '\n';

#define NON_COPYABLE(Class)                  \
    Class(const Class&) = delete;            \
    Class& operator=(const Class&) = delete; \
    Class(Class&&) = delete;                 \
    Class& operator=(Class&&) = delete;

#ifdef __PRETTY_FUNCTION__
#    define LBC_FUNCTION __PRETTY_FUNCTION__
#else
#    define LBC_FUNCTION __FUNCTION__
#endif

namespace lbc {

inline llvm::StringRef view_to_stringRef(const string_view& view) {
    return llvm::StringRef(view.data(), view.size());
}

[[noreturn]] void fatalError(const string& message);

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
    NON_COPYABLE(ValueRestorer)

    explicit ValueRestorer(T& value) : m_target{ value }, m_value{ value } {}

    // restore
    ~ValueRestorer() {
        m_target = m_value;
    }

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
