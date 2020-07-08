//
// Created by Albert on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast.def.h"

namespace lbc {

AST_FORWARD_DECLARE()

class AstVisitor {
    NON_COPYABLE(AstVisitor)
public:
    AstVisitor() = default;
    virtual ~AstVisitor() = default;
    #define VIRTUAL_VISIT(KIND) virtual void visit(Ast##KIND* ast) = 0;
    AST_CONTENT_NODES(VIRTUAL_VISIT)
    #undef VIRTUAL_VISIT
};

} // namespace lbc