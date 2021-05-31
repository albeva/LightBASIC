//
// Created by Albert Varaksin on 28/05/2021.
//
#pragma once
#include "Ast/Ast.hpp"
#include "Builder.hpp"
#include "Gen/CodeGen.hpp"
#include "Gen/ValueHandler.hpp"

namespace lbc::Gen {

class ForStmtBuilder final : Builder<AstForStmt> {
public:
    ForStmtBuilder(CodeGen& codeGen, AstForStmt& ast);

private:
    void declareVars();
    void build();
    void createBlocks();
    void configureStep();
    void checkDirection();

    void makeCondition(bool incr);
    void makeIteration(bool incr, llvm::BasicBlock* branch);

    const AstForStmt::Direction m_direction;

    const TypeRoot* m_type = nullptr;
    llvm::Type* m_llvmType = nullptr;
    llvm::Value* m_isDecr = nullptr;

    llvm::BasicBlock* m_condBlock{};
    llvm::BasicBlock* m_bodyBlock{};
    llvm::BasicBlock* m_iterBlock{};
    llvm::BasicBlock* m_exitBlock{};

    ValueHandler m_iterator{};
    ValueHandler m_limit{};
    ValueHandler m_step{};
};

} // namespace lbc::Gen
