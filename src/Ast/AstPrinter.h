//
// Created by Albert Varaksin on 22/07/2020.
//
#pragma once
#include "pch.h"
#include "AstVisitor.h"

namespace lbc {

class Context;

class AstPrinter final : public AstVisitor<AstPrinter> {
public:
    explicit AstPrinter(Context& context, llvm::raw_ostream& os) noexcept;

    AST_VISITOR_DECLARE_CONTENT_FUNCS()

private:
    void writeHeader(AstRoot* ast) noexcept;
    void writeLocation(AstRoot* ast) noexcept;
    void writeAttributes(AstAttributeList* ast) noexcept;
    void writeStmts(AstStmtList* ast) noexcept;
    void writeExpr(AstExpr* ast, StringRef name = "expr") noexcept;
    void writeIdent(AstIdentExpr* ast) noexcept;
    void writeType(AstTypeExpr* ast) noexcept;

    Context& m_context;
    llvm::json::OStream m_json;
};

} // namespace lbc
