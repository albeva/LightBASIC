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
    auto* value = gen.visit(expr).getValue();
    auto* var = gen.getBuilder().CreateAlloca(
        expr.type->getLlvmType(gen.getContext()),
        nullptr,
        name);
    gen.getBuilder().CreateStore(value, var);
    return { &gen, { var, true } };
}

ValueHandler ValueHandler::createTempOrConstant(CodeGen& gen, AstExpr& expr, StringRef name) noexcept {
    auto* value = gen.visit(expr).getValue();
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
    if (ident.next) {
        PointerUnion::operator=(&ident);
    } else {
        PointerUnion::operator=(ident.symbol);
    }
}

llvm::Value* ValueHandler::getValue() noexcept {
    if (is<ValuePtr>()) {
        auto ptr = PointerUnion::get<ValuePtr>();
        if (ptr.getInt()) {
            return m_gen->getBuilder().CreateLoad(ptr.getPointer());
        }
        return ptr.getPointer();
    }

    if (auto* symbol = this->dyn_cast<Symbol*>()) {
        if (symbol->type()->isFunction()) {
            return symbol->getLlvmValue();
        }

        return m_gen->getBuilder().CreateLoad(
            symbol->getLlvmValue(),
            symbol->identifier());
    }

    if (auto* ast = dyn_cast<AstIdentExpr*>()) {
        auto* addr = ast->symbol->getLlvmValue();
        addr = m_gen->getBuilder().CreateLoad(addr);
        if (!ast->next) {
            return addr;
        }

        std::vector<unsigned int> idxs;
        while (ast->next) {
            idxs.push_back(ast->next->symbol->getIndex());
            ast = ast->next.get();
        }
        return m_gen->getBuilder().CreateExtractValue(addr, idxs);
    }

    llvm_unreachable("Unknown ValueHandler type");
}

llvm::Value* ValueHandler::getAddress() noexcept {
    if (is<ValuePtr>()) {
        auto ptr = PointerUnion::get<ValuePtr>();
        return ptr.getPointer();
    }

    if (auto* symbol = this->dyn_cast<Symbol*>()) {
        return symbol->getLlvmValue();
    }

    if (auto* ast = dyn_cast<AstIdentExpr*>()) {
        auto& builder = m_gen->getBuilder();
        auto* addr = ast->symbol->getLlvmValue();
        while (ast->next) {
            auto* symbol = ast->next->symbol;
            addr = builder.CreateStructGEP(
                ast->symbol->type()->getLlvmType(m_gen->getContext()),
                addr,
                symbol->getIndex(),
                symbol->identifier() + ".addr");
            ast = ast->next.get();
        }
        return addr;
    }

    llvm_unreachable("Unknown ValueHandler type");
}

void ValueHandler::set(llvm::Value* val) noexcept {
    if (auto* symbol = dyn_cast<Symbol*>()) {
        m_gen->getBuilder().CreateStore(val, symbol->getLlvmValue());
        return;
    }

    if (!is<ValuePtr>()) {
        llvm_unreachable("Unknown ValueHandler type");
    }

    auto ptr = PointerUnion::get<ValuePtr>();
    auto* value = ptr.getPointer();

    if (ptr.getInt()) {
        m_gen->getBuilder().CreateStore(val, value);
        return;
    }

    if (isa<llvm::Constant>(value)) {
        if (!isa<llvm::Constant>(val)) {
            fatalError("Setting non constant to constanrt");
        }
        PointerUnion::operator=({ val, false });
        return;
    }
}
