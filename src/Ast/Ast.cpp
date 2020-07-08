//
// Created by Albert on 05/07/2020.
//
#include "Ast.h"
#include "AstVisitor.h"
#include "Lexer/Token.h"
#include "Symbol/Symbol.h"
#include "Symbol/SymbolTable.h"
using namespace lbc;

AstRoot::~AstRoot() {}

#define IMPLEMENT_AST(KIND) \
    Ast##KIND::Ast##KIND() : Base{AstKind::KIND} {} \
    void Ast##KIND::accept(AstVisitor* visitor) {   \
        visitor->visit(this);                       \
    }                                               \
    Ast##KIND::~Ast##KIND() {}

AST_CONTENT_NODES(IMPLEMENT_AST)

#undef IMPLEMENT_AST
