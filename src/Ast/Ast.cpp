//
// Created by Albert on 05/07/2020.
//
#include "Ast.h"
#include "AstVisitor.h"
#include "Lexer/Token.h"
#include "Symbol/Symbol.h"
#include "Symbol/SymbolTable.h"
using namespace lbc;

AstRoot::~AstRoot() = default;

#define IMPLEMENT_AST(KIND) \
     Ast##KIND::Ast##KIND(): Base{AstKind::KIND} {} \
     Ast##KIND::~Ast##KIND() = default;             \
     void Ast##KIND::accept(AstVisitor* visitor) {  \
         visitor->visit(this);                      \
     }
AST_CONTENT_NODES(IMPLEMENT_AST)

