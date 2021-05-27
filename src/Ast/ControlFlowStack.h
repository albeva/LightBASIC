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
 * Each control structure can have data (e.g. control exit and continue labels)
 */
template<
    typename Data = std::monostate,
    std::enable_if_t<std::is_trivial_v<Data>, int> = 0>
class ControlFlowStack final {
public:
    using Entry = std::pair<ControlFlowStatement, Data>;
    using Container = std::vector<Entry>;
    using const_iterator = typename Container::const_reverse_iterator;

    NO_COPY_AND_MOVE(ControlFlowStack)
    ControlFlowStack() noexcept = default;
    ~ControlFlowStack() noexcept = default;

    void push(ControlFlowStatement control, Data data = {}) noexcept {
        m_container.emplace_back(control, data);
    }

    void pop() {
        m_container.pop_back();
    }

    [[nodiscard]] const_iterator find(const std::vector<ControlFlowStatement>& destination) const noexcept {
        auto iter = cbegin();
        auto target = iter;
        for (auto control : destination) {
            iter = find(iter, control);
            if (iter == cend()) {
                return cend();
            }
            target = iter;
            iter++;
        }
        return target;
    }

    [[nodiscard]] const_iterator find(const_iterator from, ControlFlowStatement control) const noexcept {
        return std::find_if(from, cend(), [&](const auto& entry) {
            return entry.first == control;
        });
    }

    [[nodiscard]] bool empty() const noexcept { return m_container.empty(); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return m_container.crbegin(); }
    [[nodiscard]] const_iterator cend() const noexcept { return m_container.crend(); }

private:
    Container m_container;
};

} // namespace lbc
