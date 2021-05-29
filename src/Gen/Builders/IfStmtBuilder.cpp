//
// Created by Albert on 28/05/2021.
//
#include "IfStmtBuilder.hpp"
#include "Driver/Context.hpp"
using namespace lbc;
using namespace Gen;

IfStmtBuilder::IfStmtBuilder(CodeGen& gen, AstIfStmt& ast) noexcept
: Builder{ gen, ast } {
    build();
}

void IfStmtBuilder::build() noexcept {
    auto* func = m_builder.GetInsertBlock()->getParent();
    auto* endBlock = llvm::BasicBlock::Create(m_llvmContext, "if.end", func);

    const auto count = m_ast.blocks.size();
    for (size_t idx = 0; idx < count; idx++) {
        const auto& block = m_ast.blocks[idx];
        llvm::BasicBlock* elseBlock = nullptr;

        for (const auto& decl : block.decls) {
            m_gen.visit(*decl);
        }

        if (block.expr) {
            auto* condition = m_gen.visit(*block.expr);

            auto* thenBlock = llvm::BasicBlock::Create(m_llvmContext, "if.then", func);
            if (idx == count - 1) {
                elseBlock = endBlock;
            } else {
                elseBlock = llvm::BasicBlock::Create(m_llvmContext, "if.else", func);
            }
            m_builder.CreateCondBr(condition, thenBlock, elseBlock);

            m_gen.switchBlock(thenBlock);
        } else {
            elseBlock = endBlock;
        }

        m_gen.visit(*block.stmt);
        m_gen.terminateBlock(endBlock);

        m_gen.switchBlock(elseBlock);
    }
}
