//
// Created by Albert on 29/05/2021.
//
#pragma once
#include "pch.hpp"

namespace lbc {
class SemanticAnalyzer;
struct AstTypeDecl;

namespace Sem {
    class TypeDeclPass final {
    public:
        TypeDeclPass(SemanticAnalyzer& sem, AstTypeDecl& ast) noexcept;

    private:
        void declareMembers() noexcept;

        SemanticAnalyzer& m_sem;
        AstTypeDecl& m_ast;
    };
} // namespace Sem
} // namespace lbc
