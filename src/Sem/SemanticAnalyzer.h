//
// Created by Albert Varaksin on 08/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/AstVisitor.h"
#include "Ast/ControlFlowStack.h"
#include "Passes/ConstantFoldingPass.h"
#include "Passes/TypePass.h"

namespace lbc {

class Token;
class Symbol;
class SymbolTable;
class TypeRoot;
class Context;

class SemanticAnalyzer final : public AstVisitor<SemanticAnalyzer> {
public:
    explicit SemanticAnalyzer(Context& context) noexcept;

    AST_VISITOR_DECLARE_CONTENT_FUNCS()
private:
    /**
     * Process expression. if `type` is provided then coerce the type (implicit cast)
     * If `literal` is true, then coerce type only for literal value. E.g. default integral
     * to `INTEGER`
     */
    void expression(unique_ptr<AstExpr>& ast, const TypeRoot* type = nullptr) noexcept;
    void arithmetic(AstBinaryExpr* ast) noexcept;
    void logical(AstBinaryExpr* ast) noexcept;
    void comparison(AstBinaryExpr* ast) noexcept;
    [[nodiscard]] bool canPerformBinary(TokenKind op, const TypeRoot* left, const TypeRoot* right) const noexcept;

    [[nodiscard]] Symbol* createNewSymbol(AstDecl* ast, StringRef id) noexcept;
    static void coerce(unique_ptr<AstExpr>& expr, const TypeRoot* type) noexcept;
    static void cast(unique_ptr<AstExpr>& ast, const TypeRoot* type) noexcept;

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
