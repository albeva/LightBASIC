//
// Created by Albert Varaksin on 05/07/2020.
//
#pragma once
#include "Ast/AstVisitor.h"
#include "Ast/ControlFlowStack.hpp"
#include "ValueHandler.hpp"

namespace lbc {
class Context;

class CodeGen final : public AstVisitor<CodeGen, Gen::ValueHandler> {
public:
    explicit CodeGen(Context& context);

    [[nodiscard]] unique_ptr<llvm::Module> getModule();

    [[nodiscard]] bool validate() const noexcept;

    [[nodiscard]] Context& getContext() noexcept { return m_context; }
    [[nodiscard]] llvm::IRBuilder<>& getBuilder() noexcept { return m_builder; }
    [[nodiscard]] llvm::ConstantInt* getTrue() noexcept { return m_constantTrue; }
    [[nodiscard]] llvm::ConstantInt* getFalse() noexcept { return m_constantFalse; }
    [[nodiscard]] auto& getControlStack() noexcept { return m_controlStack; }

    void addBlock();
    void terminateBlock(llvm::BasicBlock* dest);
    void switchBlock(llvm::BasicBlock* block);

    AST_VISITOR_DECLARE_CONTENT_FUNCS()
private:
    enum class Scope {
        Root,
        Function
    };

    llvm::BasicBlock* getGlobalCtorBlock();

    void declareFuncs();
    void declareFunc(AstFuncDecl& ast);
    void declareGlobalVar(AstVarDecl& ast);
    void declareLocalVar(AstVarDecl& ast);
    llvm::Constant* getStringConstant(StringRef str);

    Context& m_context;
    llvm::LLVMContext& m_llvmContext;
    AstModule* m_astRootModule = nullptr;
    unsigned int m_fileId = ~0U;
    Scope m_scope = Scope::Root;
    unique_ptr<llvm::Module> m_module;
    llvm::Function* m_globalCtorFunc = nullptr;
    llvm::IRBuilder<> m_builder;
    llvm::StringMap<llvm::Constant*> m_stringLiterals;

    llvm::ConstantInt* m_constantFalse;
    llvm::ConstantInt* m_constantTrue;

    bool m_declareAsGlobals = true;

    struct ControlEntry final {
        llvm::BasicBlock* continueBlock;
        llvm::BasicBlock* exitBlock;
    };
    ControlFlowStack<ControlEntry> m_controlStack;
};

} // namespace lbc
