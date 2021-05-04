//
// Created by Albert Varaksin on 05/07/2020.
//
#include "Ast.h"
#include "AstVisitor.h"
using namespace lbc;

namespace literals {
constexpr std::array nodes {
#define KIND_ENUM(id, ...) llvm::StringLiteral{ "Ast" #id },
    AST_CONTENT_NODES(KIND_ENUM)
#undef KIND_ENUM
};
} // namespace literals

const llvm::StringLiteral& AstRoot::describe() const noexcept {
    auto index = static_cast<size_t>(kind());
    assert(index < literals::nodes.size());
    return literals::nodes[index];
}

const Token* AstAttributeList::getStringLiteral(const StringRef& key) const {
    for (const auto& attr : attribs) {
        if (attr->identExpr->token->lexeme() == key) {
            if (attr->argExprs.size() != 1) {
                fatalError("Attribute "_t + key + " must have 1 llvmValue", false);
            }
            const auto* token = attr->argExprs[0]->token.get();
            if (token->kind() != TokenKind::StringLiteral) {
                fatalError("Attribute "_t + key + " must be a string literal", false);
            }
            return token;
        }
    }
    return nullptr;
}
