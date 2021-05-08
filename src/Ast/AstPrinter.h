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
    explicit AstPrinter(Context& context, llvm::raw_ostream& os) noexcept
    : m_context{context}, m_os{ os } {}

    AST_VISITOR_DECLARE_CONTENT_FUNCS()

private:
    [[nodiscard]] string indent() const noexcept;
    [[nodiscard]] string range(AstRoot* pList) const noexcept;

    Context& m_context;
    llvm::raw_ostream& m_os;

    size_t m_indent = 0;
    static constexpr auto SPACES = 2;

};

} // namespace lbc
