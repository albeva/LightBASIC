//
// Created by Albert on 05/07/2020.
//
#pragma once
#include "Ast.def.h"

namespace lbc {

AST_FORWARD_DECLARE()

class AstVisitor {
public:
    #define VIRTUAL_VISIT(KIND) virtual void visit(const Ast##KIND* ast) = 0;
    AST_CONTENT_NODES(VIRTUAL_VISIT)
    #undef VIRTUAL_VISIT
};

} // namespace lbc