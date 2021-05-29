//
// Created by Albert Varaksin on 08/07/2020.
//
#pragma once
#include "pch.hpp"
#include "Ast/AstVisitor.h"
#include "Ast/ControlFlowStack.hpp"
#include "Passes/ConstantFoldingPass.hpp"
#include "Passes/TypePass.hpp"

namespace lbc {

class Token;
class Symbol;
class SymbolTable;
class TypeRoot;
class Context;

class SemanticAnalyzer final : public AstVisitor<SemanticAnalyzer> {
public:
    explicit SemanticAnalyzer(Context& context) noexcept;

    [[nodiscard]] Context& getContext() noexcept { return m_context; }

    /// analyze the expression, optionally coerce result to given type
    void expression(unique_ptr<AstExpr>& ast, const TypeRoot* type = nullptr) noexcept;

    /// Checks types and if they are convertible, create CAST expression
    static void coerce(unique_ptr<AstExpr>& expr, const TypeRoot* type) noexcept;

    /// Cast expression and fold the value
    void convert(unique_ptr<AstExpr>& ast, const TypeRoot* type) noexcept;

    /// Creates a CAST expression, without folding
    static void cast(unique_ptr<AstExpr>& ast, const TypeRoot* type) noexcept;

    [[nodiscard]] SymbolTable* getSymbolTable() noexcept { return m_table; }
    void setSymbolTable(SymbolTable* table) { m_table = table; }

    [[nodiscard]] auto& getControlStack() noexcept { return m_controlStack; }

    AST_VISITOR_DECLARE_CONTENT_FUNCS()
private:
    [[nodiscard]] Symbol* createNewSymbol(AstDecl* ast, StringRef id) noexcept;

    void arithmetic(AstBinaryExpr* ast) noexcept;
    void logical(AstBinaryExpr* ast) noexcept;
    void comparison(AstBinaryExpr* ast) noexcept;
    [[nodiscard]] bool canPerformBinary(TokenKind op, const TypeRoot* left, const TypeRoot* right) const noexcept;

    Context& m_context;
    unsigned int m_fileId = ~0U;
    AstModule* m_astRootModule = nullptr;
    AstFuncDecl* m_function = nullptr;
    SymbolTable* m_table = nullptr;
    SymbolTable* m_rootTable = nullptr;
    Sem::ConstantFoldingPass m_constantFolder;
    Sem::TypePass m_typePass;

    ControlFlowStack<> m_controlStack;
};

} // namespace lbc
