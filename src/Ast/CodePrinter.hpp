////
//// Created by Albert Varaksin on 05/07/2020.
////
#pragma once
#include "pch.hpp"
#include "Ast.hpp"
#include "AstVisitor.h"

namespace lbc {

class CodePrinter final : public AstVisitor<CodePrinter> {
public:
    explicit CodePrinter(llvm::raw_ostream& os) noexcept : m_os{ os } {}
    AST_VISITOR_DECLARE_CONTENT_FUNCS()

private:
    [[nodiscard]] string indent() const noexcept;
    size_t m_indent = 0;
    llvm::raw_ostream& m_os;
    static constexpr auto SPACES = 4;
};

} // namespace lbc