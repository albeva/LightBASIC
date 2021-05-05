//
// Created by Albert Varaksin on 05/05/2021.
//
#pragma once
#include "pch.h"

namespace lbc {

class Context;

namespace Sem {

    class ConstantFoldingPass : private NonCopyable {
    public:
        explicit ConstantFoldingPass(Context& context) noexcept : m_context{ context } {}

    private:
        Context& m_context;
    };

} // namespace Sem
} // namespace lbc
