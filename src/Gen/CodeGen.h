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
    CodeGen(Context& context);

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

    using LiteralMap = std::unordered_map<string_view, llvm::Constant*>;

    static llvm::Value* getStoreValue(AstIdentExpr* identExpr);

    Context& m_context;
    llvm::LLVMContext& m_llvmContext;
    AstModule* m_astRootModule = nullptr;
    unsigned int m_fileId = ~0U;
    Scope m_scope = Scope::Root;
    llvm::IRBuilder<> m_builder;
    unique_ptr<llvm::Module> m_module;
    llvm::Value* m_value = nullptr;
    llvm::Function* m_function = nullptr;
    llvm::BasicBlock* m_block = nullptr;
    LiteralMap m_stringLiterals;
};

} // namespace lbc
