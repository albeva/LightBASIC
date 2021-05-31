//
// Created by Albert on 28/05/2021.
//
#pragma once
#include "pch.hpp"
#include "Ast/Ast.hpp"
#include "Builder.hpp"
#include "Gen/CodeGen.hpp"

namespace lbc::Gen {

class IfStmtBuilder final : Builder<AstIfStmt> {
public:
    IfStmtBuilder(CodeGen& gen, AstIfStmt& ast);

private:
    void build();
};

} // namespace lbc::Gen
