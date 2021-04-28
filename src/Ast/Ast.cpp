//
// Created by Albert Varaksin on 05/07/2020.
//
#include "Ast.h"
#include "AstVisitor.h"
using namespace lbc;

const Token* AstAttributeList::getStringLiteral(const string_view& key) const {
    for (const auto& attr : attribs) {
        if (attr->identExpr->token->lexeme() == key) {
            if (attr->argExprs.size() != 1) {
                fatalError("Attribute "s + string(key) + " must have 1 llvmValue", false);
            }
            const auto* token = attr->argExprs[0]->token.get();
            if (token->kind() != TokenKind::StringLiteral) {
                fatalError("Attribute "s + string(key) + " must be a string literal", false);
            }
            return token;
        }
    }
    return nullptr;
}
