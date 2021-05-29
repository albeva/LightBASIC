//
// Created by Albert on 28/05/2021.
//
#pragma once
#include "pch.hpp"
#include "Ast/Ast.hpp"
#include "Gen/CodeGen.hpp"
#include "Builder.hpp"

namespace lbc::Gen {

class IfStmtBuilder final: Builder<AstIfStmt> {
public:
    IfStmtBuilder(CodeGen& gen, AstIfStmt* ast) noexcept;

private:
    void build() noexcept;
};

} // namespace lbc::Gen
