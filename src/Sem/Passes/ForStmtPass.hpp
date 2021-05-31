//
// Created by Albert on 29/05/2021.
//
#pragma once


namespace lbc {
class SemanticAnalyzer;
struct AstForStmt;

namespace Sem {
    class ForStmtPass final {
    public:
        ForStmtPass(SemanticAnalyzer& sem, AstForStmt& ast);

    private:
        void ceclare();
        void analyze();
        void determineForDirection();

        SemanticAnalyzer& m_sem;
        AstForStmt& m_ast;
    };
} // namespace Sem
} // namespace lbc
