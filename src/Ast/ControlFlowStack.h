//
// Created by Albert Varaksin on 25/05/2021.
//
#pragma once
#include "pch.h"

namespace lbc {

enum class ControlFlowStatement : uint8_t {
    For,
    Do
};

/**
 * Helper class to manage nested control structures.
 *
 * This class acts as a stack, we can push & pop structures from it
 *
 * Each control structure can have a key (e.g. symbol) and data (e.g. control exit and continue labels)
 */
template<
    typename Key,
    typename Data = std::monostate,
    std::enable_if_t<std::is_trivial_v<Key> && std::is_trivial_v<Data>, int> = 0>
class ControlFlowStack final {
public:
    struct Entry final {
        ControlFlowStatement control;
        Key key;
        Data data;
    };
    using container = std::vector<Entry>;
    using const_iterator = typename container::const_reverse_iterator;

    NO_COPY_AND_MOVE(ControlFlowStack)
    ControlFlowStack() noexcept = default;
    ~ControlFlowStack() noexcept = default;

    void push(ControlFlowStatement control, Key key = {}, Data data = {}) noexcept {
        m_vector.emplace_back(Entry{ control, key, data });
    }

    void pop() {
        m_vector.pop_back();
    }

    [[nodiscard]] bool contains(ControlFlowStatement control) const noexcept {
        return find(cbegin(), control) != cend();
    }

    [[nodiscard]] bool contains(ControlFlowStatement control, Key key) const noexcept {
        return find(cbegin(), control, key) != cend();
    }

    [[nodiscard]] const_iterator find(ControlFlowStatement control) const noexcept {
        return find(cbegin(), control);
    }

    [[nodiscard]] const_iterator find(const_iterator from, ControlFlowStatement control) const noexcept {
        return std::find_if(from, cend(), [&](const auto& entry) {
            return entry.control == control;
        });
    }

    [[nodiscard]] const_iterator find(ControlFlowStatement control, Key key) const noexcept {
        return find(cbegin(), control, key);
    }

    [[nodiscard]] const_iterator find(const_iterator from, ControlFlowStatement control, Key key) const noexcept {
        return std::find_if(from, cend(), [&](const auto& entry) {
            return entry.control == control && entry.key == key;
        });
    }

    [[nodiscard]] const_iterator cbegin() const noexcept { return m_vector.crbegin(); }
    [[nodiscard]] const_iterator cend() const noexcept { return m_vector.crend(); }

private:
    container m_vector;
};

} // namespace lbc
