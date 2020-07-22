//
// Created by albert on 22/07/2020.
//
#pragma once
#include "pch.h"
#include "AstVisitor.h"

namespace lbc {

class AstPrinter final : public AstVisitor<AstPrinter> {
public:
    explicit AstPrinter(llvm::raw_ostream& os) : m_os{ os } {}
    AST_DECLARE_ALL_ROOT_VISIT_METHODS()

private:
    [[nodiscard]] string indent() const;
    size_t m_indent = 0;
    llvm::raw_ostream& m_os;
    static constexpr auto SPACES = 2;
};

} // namespace lbc
