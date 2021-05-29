//
// Created by Albert Varaksin on 05/07/2020.
//
#pragma once
#include "pch.hpp"
#include "Ast/AstVisitor.h"
#include "Ast/ControlFlowStack.hpp"

namespace lbc {

class Context;

class CodeGen final : public AstVisitor<CodeGen, llvm::Value*> {
public:
    explicit CodeGen(Context& context) noexcept;

    [[nodiscard]] unique_ptr<llvm::Module> getModule() noexcept;

    [[nodiscard]] bool validate() const noexcept;

    [[nodiscard]] Context& getContext() noexcept { return m_context; }

    [[nodiscard]] llvm::IRBuilder<>& getBuilder() noexcept { return m_builder; }

    [[nodiscard]] llvm::ConstantInt* getTrue() noexcept { return m_constantTrue; }
    [[nodiscard]] llvm::ConstantInt* getFalse() noexcept { return m_constantFalse; }

    void addBlock() noexcept;

    void terminateBlock(llvm::BasicBlock* dest) noexcept;

    void switchBlock(llvm::BasicBlock* block) noexcept;

    [[nodiscard]] auto& getControlStack() noexcept {
        return m_controlStack;
    }

    AST_VISITOR_DECLARE_CONTENT_FUNCS()
private:
    enum class Scope {
        Root,
        Function
    };

    llvm::BasicBlock* getGlobalCtorBlock() noexcept;

    void declareFuncs() noexcept;
    void declareFunc(AstFuncDecl* ast) noexcept;
    void declareGlobalVar(AstVarDecl* ast) noexcept;
    void declareLocalVar(AstVarDecl* ast) noexcept;
    static llvm::Value* getStoreValue(AstExpr* identExpr) noexcept;
    llvm::Value* getStringConstant(StringRef str) noexcept;

    Context& m_context;
    llvm::LLVMContext& m_llvmContext;
    AstModule* m_astRootModule = nullptr;
    unsigned int m_fileId = ~0U;
    Scope m_scope = Scope::Root;
    unique_ptr<llvm::Module> m_module;
    llvm::Function* m_globalCtorFunc = nullptr;
    llvm::IRBuilder<> m_builder;
    llvm::StringMap<llvm::Constant*> m_stringLiterals;

    llvm::ConstantInt* m_constantFalse = nullptr;
    llvm::ConstantInt* m_constantTrue = nullptr;

    bool m_declareAsGlobals = true;

    struct ControlEntry final {
        llvm::BasicBlock* continueBlock;
        llvm::BasicBlock* exitBlock;
    };
    ControlFlowStack<ControlEntry> m_controlStack;
};

} // namespace lbc
