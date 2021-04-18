//
// Created by Albert on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"

namespace lbc {

class Context;

class CodeGen final : public AstVisitor<CodeGen> {
public:
    CodeGen(Context& context, unsigned fileId);

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
    using ValueMap = std::unordered_map<string, llvm::Value*>;
    using LiteralMap = std::unordered_map<string_view, llvm::Constant*>;

    static llvm::Value* getStoreValue(AstIdentExpr* identExpr);

    Context& m_context;
    llvm::LLVMContext& m_llvmContext;
    unsigned m_fileId;

    llvm::IRBuilder<> m_builder;
    unique_ptr<llvm::Module> m_module;
    llvm::Value* m_value = nullptr;
    llvm::Function* m_function = nullptr;
    llvm::Function* m_startupFn = nullptr;
    llvm::BasicBlock* m_startupBlock = nullptr;

    llvm::BasicBlock* m_block = nullptr;
    ValueMap m_values;
    LiteralMap m_stringLiterals;
};

} // namespace lbc
