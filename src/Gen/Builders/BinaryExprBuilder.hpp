//
// Created by Albert on 28/05/2021.
//
#pragma once
#include "Ast/Ast.hpp"
#include "Builder.hpp"
#include "Gen/CodeGen.hpp"
#include "Gen/ValueHandler.hpp"

namespace lbc::Gen {

class BinaryExprBuilder final : Builder<AstBinaryExpr> {
public:
    using Builder::Builder;
    ValueHandler build();

private:
    ValueHandler comparison();
    ValueHandler arithmetic();
    ValueHandler logical();
};

} // namespace lbc::Gen
