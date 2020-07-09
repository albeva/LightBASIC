//
// Created by Albert on 03/07/2020.
//
#pragma once

#define LOG_VAR(VAR) std::cout << #VAR << " = " << VAR << '\n';

namespace lbc {

inline llvm::StringRef view_to_stringRef(const string_view& view) {
    return llvm::StringRef(view.data(), view.size());
}

#define NON_COPYABLE(Class) \
    Class(const Class&) = delete;               \
    Class& operator=(const Class&) = delete;    \
    Class(Class&&) = delete;                    \
    Class& operator=(Class&&) = delete;

/**
 * Simple helper. Basically ensured that original
 * value of a variable provided is restored apon
 * exiting the scope
 */
template<typename T, std::enable_if_t<
    std::is_trivially_copyable_v<T> &&
    std::is_trivially_assignable_v<T&, T>, int> = 0>
struct ValueRestorer {
    NON_COPYABLE(ValueRestorer)

    explicit ValueRestorer(T& value): m_target{value}, m_value{value} {}

    // restore
    ~ValueRestorer() {
        m_target = m_value;
    }

    // members
private:
    T & m_target;
    T   m_value;
};

#define CONCATENATE_DETAIL(x, y) x##y
#define CONCATENATE(x, y) CONCATENATE_DETAIL(x, y)
#define MAKE_UNIQUE(x) CONCATENATE(x, __COUNTER__)
#define RESTORE_ON_EXIT(V) ValueRestorer<decltype(V)> MAKE_UNIQUE(tmp_restore_onexit_){V};

}