//
// Created by Albert Varaksin on 28/05/2021.
//
#include "ForStmtBuilder.hpp"
#include "Ast/Ast.hpp"
#include "Driver/Context.hpp"
#include "Gen/CodeGen.hpp"
#include "Gen/Helpers.hpp"
#include "Gen/ValueHandler.hpp"
#include "Symbol/Symbol.hpp"
#include "Type/Type.hpp"
using namespace lbc;
using namespace Gen;

ForStmtBuilder::ForStmtBuilder(CodeGen& codeGen, AstForStmt* ast) noexcept
: m_gen{ codeGen },
  m_builder{ codeGen.getBuilder() },
  m_llvmContext{ codeGen.getContext().getLlvmContext() },
  m_ast{ ast },
  m_direction{ ast->direction } {
    if (m_direction == AstForStmt::Direction::Skip) {
        return;
    }

    createBlocks();
    declareVars();
    checkDirection();
    m_step = getStep();
    build();
}

void ForStmtBuilder::declareVars() noexcept {
    for (const auto& decl : m_ast->decls) {
        m_gen.visit(decl.get());
    }
    m_gen.visit(m_ast->iterator.get());

    m_type = m_ast->iterator->symbol->type();
    m_llvmType = m_type->getLlvmType(m_gen.getContext());

    m_iterator = ValueHandler{ &m_gen, m_ast->iterator->symbol };
    m_limit = ValueHandler(&m_gen, m_ast->limit.get(), "for.limit");
}

void ForStmtBuilder::checkDirection() noexcept {
    if (m_direction == AstForStmt::Direction::Unknown) {
        auto* limitValue = m_limit.get();
        auto* iterValue = m_iterator.get();
        m_isDecr = m_builder.CreateCmp(
            getCmpPred(m_type, TokenKind::LessThan),
            limitValue,
            iterValue,
            "for.isdecr");
    }
}

void ForStmtBuilder::createBlocks() noexcept {
    m_condBlock = llvm::BasicBlock::Create(m_llvmContext, "for.cond");
    m_bodyBlock = llvm::BasicBlock::Create(m_llvmContext, "for.body");
    m_iterBlock = llvm::BasicBlock::Create(m_llvmContext, "for.iter");
    m_exitBlock = llvm::BasicBlock::Create(m_llvmContext, "for.end");
}

ValueHandler ForStmtBuilder::getStep() noexcept {
    // No step
    if (!m_ast->step) {
        llvm::Constant* stepVal = nullptr;
        if (const auto* integral = dyn_cast<TypeIntegral>(m_type)) {
            stepVal = llvm::ConstantInt::get(m_llvmType, 1, integral->isSigned());
        } else if (isa<TypeFloatingPoint>(m_type)) {
            stepVal = llvm::ConstantFP::get(m_llvmType, 1);
        } else {
            llvm_unreachable("Unknown type");
        }
        return ValueHandler{ &m_gen, stepVal };
    }

    // Literal value
    if (auto* literal = dyn_cast<AstLiteralExpr>(m_ast->step.get())) {
        const auto* stepTy = literal->type;
        llvm::Constant* stepVal = nullptr;
        if (const auto* integral = dyn_cast<TypeIntegral>(stepTy)) {
            auto stepLit = std::get<uint64_t>(literal->value);
            if (integral->isSigned()) {
                auto sstepLit = static_cast<int64_t>(stepLit);
                if (sstepLit < 0) {
                    stepLit = static_cast<uint64_t>(-sstepLit);
                }
            }
            stepVal = llvm::ConstantInt::get(m_llvmType, stepLit, false);
        } else if (isa<TypeFloatingPoint>(stepTy)) {
            auto stepLit = std::get<double>(literal->value);
            if (stepLit < 0) {
                stepLit = -stepLit;
            }
            stepVal = llvm::ConstantFP::get(m_llvmType, stepLit);
        } else {
            llvm_unreachable("Unkown type");
        }
        return ValueHandler{ &m_gen, stepVal };
    }

    // Unknown value
    auto step = ValueHandler{ &m_gen, m_ast->step.get(), "__for.step" };
    auto* stepValue = step.get();

    auto* isStepNeg = m_builder.CreateCmp(
        getCmpPred(m_ast->step->type, TokenKind::LessThan),
        stepValue,
        llvm::Constant::getNullValue(m_llvmType),
        "for.isStepNeg");

    auto* negateBlock = llvm::BasicBlock::Create(m_llvmContext, "for.step.negate");

    switch (m_direction) {
    case AstForStmt::Direction::Unknown: {
        auto* isDecrBlock = llvm::BasicBlock::Create(m_llvmContext, "for.step.decr");
        auto* isIncrBlock = llvm::BasicBlock::Create(m_llvmContext, "for.step.incr");
        m_builder.CreateCondBr(m_isDecr, isDecrBlock, isIncrBlock);

        m_gen.switchBlock(isDecrBlock);
        m_builder.CreateCondBr(isStepNeg, negateBlock, m_exitBlock);

        m_gen.switchBlock(isIncrBlock);
        m_builder.CreateCondBr(isStepNeg, m_exitBlock, m_condBlock);
        break;
    }
    case AstForStmt::Direction::Skip:
        break;
    case AstForStmt::Direction::Increment:
        m_builder.CreateCondBr(isStepNeg, m_exitBlock, m_condBlock);
        break;
    case AstForStmt::Direction::Decrement:
        m_builder.CreateCondBr(isStepNeg, negateBlock, m_exitBlock);
        break;
    }

    m_gen.switchBlock(negateBlock);
    stepValue = m_builder.CreateNeg(stepValue);
    step.set(stepValue);
    m_builder.CreateBr(m_condBlock);

    return step;
}

void ForStmtBuilder::build() noexcept {
    llvm::BasicBlock* incrBlock = nullptr;
    llvm::BasicBlock* decrBlock = nullptr;

    // Condition
    m_gen.switchBlock(m_condBlock);
    switch (m_direction) {
    case AstForStmt::Direction::Unknown: {
        incrBlock = llvm::BasicBlock::Create(m_llvmContext, "for.cond.incr");
        decrBlock = llvm::BasicBlock::Create(m_llvmContext, "for.cond.decr");
        m_builder.CreateCondBr(m_isDecr, decrBlock, incrBlock);

        m_gen.switchBlock(incrBlock);
        makeCondition(true);

        m_gen.switchBlock(decrBlock);
        makeCondition(false);
        break;
    }
    case AstForStmt::Direction::Skip:
        break;
    case AstForStmt::Direction::Increment:
        makeCondition(true);
        break;
    case AstForStmt::Direction::Decrement:
        makeCondition(false);
        break;
    }

    // Body
    m_gen.switchBlock(m_bodyBlock);
    m_gen.getControlStack().push(ControlFlowStatement::For, { m_iterBlock, m_exitBlock });
    m_gen.visit(m_ast->stmt.get());
    m_gen.getControlStack().pop();

    // Iteration
    m_gen.switchBlock(m_iterBlock);
    switch (m_direction) {
    case AstForStmt::Direction::Unknown: {
        auto* iterIncrBlock = llvm::BasicBlock::Create(m_llvmContext, "for.iter.incr");
        auto* iterDecrBlock = llvm::BasicBlock::Create(m_llvmContext, "for.iter.decr");
        m_builder.CreateCondBr(m_isDecr, iterDecrBlock, iterIncrBlock);

        m_gen.switchBlock(iterIncrBlock);
        makeIteration(true);
        m_builder.CreateBr(incrBlock);

        m_gen.switchBlock(iterDecrBlock);
        makeIteration(false);
        m_builder.CreateBr(decrBlock);
        break;
    }
    case AstForStmt::Direction::Skip:
        break;
    case AstForStmt::Direction::Increment:
        makeIteration(true);
        m_builder.CreateBr(m_condBlock);
        break;
    case AstForStmt::Direction::Decrement:
        makeIteration(false);
        m_builder.CreateBr(m_condBlock);
        break;
    }

    // End
    m_gen.switchBlock(m_exitBlock);
}

void ForStmtBuilder::makeCondition(bool incr) noexcept {
    auto lessOrEqualPred = getCmpPred(m_type, TokenKind::LessOrEqual);
    auto* iterValue = m_iterator.get();
    auto* limitValue = m_limit.get();
    auto* cmp = incr
        ? m_builder.CreateCmp(lessOrEqualPred, iterValue, limitValue, "for.incrCond")
        : m_builder.CreateCmp(lessOrEqualPred, limitValue, iterValue, "for.decrCond");

    m_builder.CreateCondBr(cmp, m_bodyBlock, m_exitBlock);
}

void ForStmtBuilder::makeIteration(bool incr) noexcept {
    auto* stepValue = m_step.get();
    auto* iterValue = m_iterator.get();
    auto* result = incr
        ? m_builder.CreateAdd(iterValue, stepValue)
        : m_builder.CreateSub(iterValue, stepValue);
    m_iterator.set(result);
}
