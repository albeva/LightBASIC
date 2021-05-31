//
// Created by Albert Varaksin on 22/07/2020.
//
#pragma once
#include "pch.hpp"
#include "AstVisitor.h"

namespace lbc {

class Context;

class AstPrinter final : public AstVisitor<AstPrinter> {
public:
    explicit AstPrinter(Context& context, llvm::raw_ostream& os) noexcept;

    AST_VISITOR_DECLARE_CONTENT_FUNCS()

private:
    void writeHeader(AstRoot& ast);
    void writeLocation(AstRoot& ast);

    void writeAttributes(AstAttributeList* ast);
    void writeStmts(AstStmtList* ast);
    void writeExpr(AstExpr* ast, StringRef name = "expr");
    void writeIdent(AstIdentExpr* ast);
    void writeType(AstTypeExpr* ast);

    Context& m_context;
    llvm::json::OStream m_json;
};

} // namespace lbc
