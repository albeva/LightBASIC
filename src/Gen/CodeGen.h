//
// Created by Albert Varaksin on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"

namespace lbc {

class Context;

class CodeGen final : public AstVisitor<CodeGen, llvm::Value*> {
public:
    explicit CodeGen(Context& context) noexcept;

    /**
     * Give up ownership of the generated module.
     */
    [[nodiscard]] unique_ptr<llvm::Module> getModule() noexcept;

    /**
     * Validate module
     */
    [[nodiscard]] bool validate() const noexcept;

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
    llvm::Value* comparison(AstBinaryExpr* ast) noexcept;
    llvm::Value* arithmetic(AstBinaryExpr* ast) noexcept;
    llvm::Value* logical(AstBinaryExpr* ast) noexcept;

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
};

} // namespace lbc
