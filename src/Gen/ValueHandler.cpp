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

ValueHandler::ValueHandler(CodeGen* gen, AstIdentExpr& ident) noexcept
: m_gen{ gen } {
    if (ident.parts.size() == 1) {
        PointerUnion::operator=(ident.parts[0].symbol);
    } else {
        PointerUnion::operator=(&ident);
    }
}

llvm::Value* ValueHandler::getAddress() noexcept {
    if (is<ValuePtr>()) {
        return PointerUnion::get<ValuePtr>().getPointer();
    }

    if (auto* symbol = this->dyn_cast<Symbol*>()) {
        return symbol->getLlvmValue();
    }

    if (auto* ast = dyn_cast<AstIdentExpr*>()) {
        auto& parts = ast->parts;
        auto& builder = m_gen->getBuilder();
        auto* symbol = ast->parts[0].symbol;

        auto* addr = symbol->getLlvmValue();
        if (symbol->type()->isPointer()) {
            addr = builder.CreateLoad(addr);
        }

        llvm::SmallVector<llvm::Value*, 4> idxs;
        idxs.reserve(ast->parts.size());
        idxs.push_back(builder.getInt64(0));
        for (size_t index = 1; index < parts.size(); index++) {
            symbol = parts[index].symbol;
            idxs.push_back(builder.getInt32(symbol->getIndex()));

            if (index + 1 < parts.size() && symbol->type()->isPointer()) {
                addr = builder.CreateGEP(addr, idxs);
                addr = builder.CreateLoad(addr);
                idxs.pop_back_n(idxs.size() - 1);
            }
        }

        return builder.CreateGEP(addr, idxs);
    }

    llvm_unreachable("Unknown ValueHandler type");
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
