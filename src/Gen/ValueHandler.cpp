//
// Created by Albert on 31/05/2021.
//
#include "ValueHandler.hpp"
#include "Ast/Ast.hpp"
#include "CodeGen.hpp"
#include "Symbol/Symbol.hpp"
#include "Type/Type.hpp"
#include <llvm/ADT/TypeSwitch.h>
using namespace lbc;
using namespace Gen;
using namespace value_handler_detail;

ValueHandler ValueHandler::createTemp(CodeGen& gen, AstExpr& expr, StringRef name) {
    auto* value = gen.visit(expr).get();
    auto* var = gen.getBuilder().CreateAlloca(
        expr.type->getLlvmType(gen.getContext()),
        nullptr,
        name);
    gen.getBuilder().CreateStore(value, var);
    return { &gen, { var, true } };
}

ValueHandler ValueHandler::createTempOrConstant(CodeGen& gen, AstExpr& expr, StringRef name) {
    auto* value = gen.visit(expr).get();
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

ValueHandler::ValueHandler(CodeGen* gen, ValuePtr ptr)
: PointerUnion{ ptr }, m_gen{ gen } {}

ValueHandler::ValueHandler(CodeGen* gen, Symbol* symbol)
: PointerUnion{ symbol }, m_gen{ gen } {}

ValueHandler::ValueHandler(CodeGen* gen, llvm::Value* value)
: PointerUnion{ ValuePtr{ value, false } }, m_gen{ gen } {}

llvm::Value* ValueHandler::get() {
    if (is<ValuePtr>()) {
        auto ptr = PointerUnion::get<ValuePtr>();
        if (ptr.getInt()) {
            return m_gen->getBuilder().CreateLoad(ptr.getPointer());
        }
        return ptr.getPointer();
    }

    if (auto* symbol = this->dyn_cast<Symbol*>()) {
        if (symbol->type()->getKind() == TypeFamily::Function) {
            return symbol->getLlvmValue();
        }
        return m_gen->getBuilder().CreateLoad(
            symbol->getLlvmValue(),
            symbol->name());
    }

    llvm_unreachable("Unknown ValueHandler type");
}

void ValueHandler::set(llvm::Value* val) {
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
