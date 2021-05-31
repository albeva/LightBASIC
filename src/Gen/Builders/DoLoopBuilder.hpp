//
// Created by Albert Varaksin on 28/05/2021.
//
#pragma once
#include "pch.hpp"
#include "Ast/Ast.hpp"
#include "Builder.hpp"
#include "Gen/CodeGen.hpp"

namespace lbc::Gen {

class DoLoopBuilder final : Builder<AstDoLoopStmt> {
public:
    DoLoopBuilder(CodeGen& gen, AstDoLoopStmt& ast);

private:
    void makeCondition(bool isUntil);
    void build();

    llvm::BasicBlock* m_bodyBlock;
    llvm::BasicBlock* m_condBlock;
    llvm::BasicBlock* m_exitBlock;
    llvm::BasicBlock* m_continueBlock;
};

} // namespace lbc::Gen
