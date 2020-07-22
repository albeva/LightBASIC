//
// Created by Albert on 05/07/2020.
//
#pragma once
#include "AstVisitor.h"

namespace lbc {

class AstPrinter final : public AstVisitor {
public:
    AST_DECLARE_ALL_VISIT_METHODS()
};

} // namespace lbc
