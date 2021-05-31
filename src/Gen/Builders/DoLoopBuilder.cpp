//
// Created by Albert Varaksin on 28/05/2021.
//
#include "DoLoopBuilder.hpp"
#include "Driver/Context.hpp"
using namespace lbc;
using namespace Gen;

DoLoopBuilder::DoLoopBuilder(CodeGen& gen, AstDoLoopStmt& ast)
: Builder{ gen, ast },
  m_bodyBlock{ llvm::BasicBlock::Create(m_llvmContext, "do_loop.body") },
  m_condBlock{ (m_ast.condition == AstDoLoopStmt::Condition::None)
          ? nullptr
          : llvm::BasicBlock::Create(m_llvmContext, "do_loop.cond") },
  m_exitBlock{ llvm::BasicBlock::Create(m_llvmContext, "do_loop.end") },
  m_continueBlock{ m_condBlock } {
    build();
}

void DoLoopBuilder::build() {
    for (const auto& decl : m_ast.decls) {
        m_gen.visit(*decl);
    }

    // pre makeCondition
    switch (m_ast.condition) {
    case AstDoLoopStmt::Condition::None:
        m_continueBlock = m_bodyBlock;
        break;
    case AstDoLoopStmt::Condition::PreUntil:
        makeCondition(true);
        break;
    case AstDoLoopStmt::Condition::PreWhile:
        makeCondition(false);
        break;
    default:
        break;
    }

    // body
    m_gen.switchBlock(m_bodyBlock);
    m_gen.getControlStack().push(ControlFlowStatement::Do, { m_continueBlock, m_exitBlock });
    m_gen.visit(*m_ast.stmt);
    m_gen.getControlStack().pop();

    // post makeCondition
    switch (m_ast.condition) {
    case AstDoLoopStmt::Condition::PostUntil:
        makeCondition(true);
        break;
    case AstDoLoopStmt::Condition::PostWhile:
        makeCondition(false);
        break;
    default:
        m_gen.terminateBlock(m_continueBlock);
        break;
    }

    // exit
    m_gen.switchBlock(m_exitBlock);
}

void DoLoopBuilder::makeCondition(bool isUntil) {
    m_gen.switchBlock(m_condBlock);
    auto value = m_gen.visit(*m_ast.expr).getValue();
    if (isUntil) {
        m_builder.CreateCondBr(value, m_exitBlock, m_bodyBlock);
    } else {
        m_builder.CreateCondBr(value, m_bodyBlock, m_exitBlock);
    }
}