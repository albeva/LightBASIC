//
// Created by Albert Varaksin on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"

namespace lbc {

class Context;

class CodeGen final : public AstVisitor<CodeGen> {
public:
    explicit CodeGen(Context& context);

    /**
     * Give up ownership of the generated module.
     */
    [[nodiscard]] unique_ptr<llvm::Module> getModule();

    /**
     * Validate module
     */
    [[nodiscard]] bool validate() const;

    /**
     * Print module
     */
    void print() const;

    AST_DECLARE_ALL_ROOT_VISIT_METHODS()

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
    static llvm::Value* getStoreValue(AstIdentExpr* identExpr);
    llvm::Constant* getStringConstant(const StringRef& str) noexcept;

    Context& m_context;
    llvm::LLVMContext& m_llvmContext;
    AstModule* m_astRootModule = nullptr;
    unsigned int m_fileId = ~0U;
    Scope m_scope = Scope::Root;
    unique_ptr<llvm::Module> m_module;
    llvm::Value* m_value = nullptr;
    llvm::Function* m_function = nullptr;
    llvm::BasicBlock* m_block = nullptr;
    llvm::BasicBlock* m_globalCtorBlock = nullptr;
    llvm::StringMap<llvm::Constant*> m_stringLiterals;
};

} // namespace lbc
