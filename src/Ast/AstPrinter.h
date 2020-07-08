//
// Created by Albert on 05/07/2020.
//
#pragma once
#include "AstVisitor.h"

namespace lbc {

class AstPrinter final: public AstVisitor {
public:
    #define IMPL_VISITOR(NODE, ...) virtual void visit(Ast##NODE* ast);
    AST_CONTENT_NODES(IMPL_VISITOR)
    #undef IMPL_VISITOR
};

} // namespace lbc
