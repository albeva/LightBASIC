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
    virtual ~AstVisitor();
#define VIRTUAL_VISIT(KIND) virtual std::any visit(Ast##KIND* ast) = 0;
    AST_CONTENT_NODES(VIRTUAL_VISIT)
#undef VIRTUAL_VISIT
};

#define AST_DECLARE_VISIT_METHOD(KIND) std::any visit(Ast##KIND* ast) final;
#define AST_DECLARE_ALL_VISIT_METHODS() \
    AST_CONTENT_NODES(AST_DECLARE_VISIT_METHOD)

} // namespace lbc
