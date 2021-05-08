//
// Created by Albert Varaksin on 22/07/2020.
//
#pragma once
#include "pch.h"
#include "AstVisitor.h"

namespace lbc {

class AstPrinter final : public AstVisitor<AstPrinter> {
public:
    explicit AstPrinter(llvm::raw_ostream& os) : m_os{ os } {}

    AST_VISITOR_DECLARE_CONTENT_FUNCS()

private:
    [[nodiscard]] string indent() const noexcept;
    size_t m_indent = 0;
    llvm::raw_ostream& m_os;
    static constexpr auto SPACES = 2;
};

} // namespace lbc
