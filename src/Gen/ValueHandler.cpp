//
// Created by Albert on 31/05/2021.
//
#include "ValueHandler.hpp"
#include "Ast/Ast.hpp"
#include "CodeGen.hpp"
#include "Symbol/Symbol.hpp"
#include "Type/Type.hpp"
#include "Type/TypeUdt.hpp"
#include <llvm/ADT/TypeSwitch.h>
using namespace lbc;
using namespace Gen;
using namespace value_handler_detail;

ValueHandler ValueHandler::createTemp(CodeGen& gen, AstExpr& expr, StringRef name) noexcept {
    auto* value = gen.visit(expr).load();
    auto* var = gen.getBuilder().CreateAlloca(
        expr.type->getLlvmType(gen.getContext()),
        nullptr,
        name);
    gen.getBuilder().CreateStore(value, var);
    return { &gen, { var, true } };
}

ValueHandler ValueHandler::createTempOrConstant(CodeGen& gen, AstExpr& expr, StringRef name) noexcept {
    auto* value = gen.visit(expr).load();
    if (isa<llvm::Constant>(value)) {
        return { &gen, { value, false } };
    }

    auto* var = gen.getBuilder().CreateAlloca(
        expr.type->getLlvmType(gen.getContext()),
        nullptr,
        name);
    gen.getBuilder().CreateStore(value, var);
    return { &gen, { var, true } };
}

ValueHandler::ValueHandler(CodeGen* gen, ValuePtr ptr) noexcept
: PointerUnion{ ptr }, m_gen{ gen } {}

ValueHandler::ValueHandler(CodeGen* gen, Symbol* symbol) noexcept
: PointerUnion{ symbol }, m_gen{ gen } {}

ValueHandler::ValueHandler(CodeGen* gen, llvm::Value* value) noexcept
: PointerUnion{ ValuePtr{ value, false } }, m_gen{ gen } {}

ValueHandler::ValueHandler(CodeGen* gen, AstIdentExpr& ast) noexcept
: PointerUnion{ ast.symbol }, m_gen{ gen } {}

ValueHandler::ValueHandler(CodeGen* gen, AstMemberAccess& ast) noexcept
: PointerUnion{ &ast }, m_gen{ gen } {}

llvm::Value* ValueHandler::getAddress() noexcept {
    if (is<ValuePtr>()) {
        return PointerUnion::get<ValuePtr>().getPointer();
    }

    if (auto* symbol = dyn_cast<Symbol*>()) {
        return symbol->getLlvmValue();
    }

    // a.b.c.d = { lhs a, { lhs b, { lhs c, rhs d }}}
    if (auto* member = dyn_cast<AstMemberAccess*>()) {
        auto& builder = m_gen->getBuilder();

        auto* lhs = m_gen->visit(*member->lhs).getAddress();
        if (member->lhs->type->isPointer()) {
            lhs = builder.CreateLoad(lhs);
        }

        llvm::SmallVector<llvm::Value*, 4> idxs;
        idxs.push_back(builder.getInt64(0));
        auto* addr = m_gen->visit(*member->rhs).getAggregateAddress(lhs, idxs, true);
        return builder.CreateGEP(addr, idxs);
    }

    llvm_unreachable("Unknown ValueHandler type");
}

llvm::Value* ValueHandler::getAggregateAddress(llvm::Value* base, IndexArray& idxs, bool terminal) noexcept {
    // end of the member access chain
    if (auto* symbol = dyn_cast<Symbol*>()) {
        auto& builder = m_gen->getBuilder();
        idxs.push_back(builder.getInt32(symbol->getIndex()));
        if (terminal && symbol->type()->isPointer()) {
            base = builder.CreateGEP(base, idxs);
            base = builder.CreateLoad(base);
            idxs.pop_back_n(idxs.size() - 1);
        }
        return base;
    }

    // middle of the chain
    if (auto* member = dyn_cast<AstMemberAccess*>()) {
        base = m_gen->visit(*member->lhs).getAggregateAddress(base, idxs, false);
        return m_gen->visit(*member->rhs).getAggregateAddress(base, idxs, true);
    }

    llvm_unreachable("Unknown aggregate member access type");
}

llvm::Value* ValueHandler::load() noexcept {
    auto* addr = getAddress();
    if (isa<llvm::Function>(addr)) {
        return addr;
    }

    if (is<ValuePtr>() && !PointerUnion::get<ValuePtr>().getInt()) {
        return addr;
    }

    return m_gen->getBuilder().CreateLoad(addr);
}

void ValueHandler::store(llvm::Value* val) noexcept {
    auto* addr = getAddress();
    m_gen->getBuilder().CreateStore(val, addr);
}
