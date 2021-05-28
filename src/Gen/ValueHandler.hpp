//
// Created by Albert Varaksin on 28/05/2021.
//
#pragma once
#include "Ast/Ast.hpp"
#include "CodeGen.hpp"
#include "Symbol/Symbol.hpp"
#include "Type/Type.hpp"

namespace lbc {

class ValueHandler final {
public:
    constexpr ValueHandler() noexcept = default;

    constexpr ValueHandler(CodeGen* gen_, AstExpr* ast, StringRef name = "") noexcept
    : gen{ gen_ },
      literal{ ast->kind == lbc::AstKind::LiteralExpr } {
        auto* expr = gen->visit(ast);
        if (literal) {
            value = expr;
            return;
        }
        value = gen->getBuilder().CreateAlloca(
            ast->type->getLlvmType(gen->getContext()),
            nullptr,
            name);
        gen->getBuilder().CreateStore(expr, value);
    }

    constexpr ValueHandler(CodeGen* gen_, llvm::Constant* constant) noexcept
    : gen{ gen_ },
      literal{ true },
      value{ constant } {}

    constexpr ValueHandler(CodeGen* gen_, Symbol* symbol) noexcept
    : gen{ gen_ },
      value{ symbol->getLlvmValue() } {}

    [[nodiscard]] llvm::Value* get() noexcept {
        if (literal) {
            return value;
        }
        return gen->getBuilder().CreateLoad(value, value->getName());
    }

    void set(llvm::Value* val) noexcept {
        if (literal) {
            if (!isa<llvm::Constant>(val)) {
                fatalError("Setting non constant to constanrt");
            }
            value = val;
            return;
        }
        gen->getBuilder().CreateStore(val, value);
    }

private:
    CodeGen* gen = nullptr;
    bool literal = false;
    llvm::Value* value = nullptr;
};

} // namespace lbc
