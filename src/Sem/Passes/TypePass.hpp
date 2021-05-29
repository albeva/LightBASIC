//
// Created by Albert Varaksin on 22/05/2021.
//
#pragma once
#include "pch.hpp"

namespace lbc {

class Context;
struct AstTypeExpr;

namespace Sem {

    class TypePass final {
    public:
        NO_COPY_AND_MOVE(TypePass)

        explicit TypePass(Context& context) noexcept : m_context{ context } {}
        ~TypePass() noexcept = default;

        void visit(AstTypeExpr& ast) noexcept;

    private:
        Context& m_context;
    };

} // namespace Sem
} // namespace lbc
