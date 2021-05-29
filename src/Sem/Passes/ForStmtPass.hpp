//
// Created by Albert on 29/05/2021.
//
#pragma once
#include "pch.hpp"

namespace lbc {
class SemanticAnalyzer;
struct AstForStmt;

namespace Sem {
    class ForStmtPass final {
    public:
        ForStmtPass(SemanticAnalyzer& sem, AstForStmt& ast) noexcept;

    private:
        void ceclare() noexcept;
        void analyze() noexcept;
        void determineForDirection() noexcept;

        SemanticAnalyzer& m_sem;
        AstForStmt& m_ast;
    };
} // namespace Sem
} // namespace lbc
