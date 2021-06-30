//
// Created by Albert Varaksin on 05/07/2020.
//
#include "Ast.hpp"
#include "AstVisitor.h"
#include "Type/Type.hpp"
using namespace lbc;

namespace literals {
constexpr std::array nodes {
#define KIND_ENUM(id, ...) llvm::StringLiteral{ "Ast" #id },
    AST_CONTENT_NODES(KIND_ENUM)
#undef KIND_ENUM
};
} // namespace literals

StringRef AstRoot::getClassName() const noexcept {
    auto index = static_cast<size_t>(kind);
    assert(index < literals::nodes.size()); // NOLINT
    return literals::nodes.at(index);
}

std::optional<StringRef> AstAttributeList::getStringLiteral(StringRef key) const noexcept {
    for (const auto& attr : attribs) {
        if (attr->identExpr->name == key) {
            if (attr->args->exprs.size() != 1) {
                fatalError("Attribute "_t + key + " must have 1 value", false);
            }
            if (auto* literal = dyn_cast<AstLiteralExpr>(attr->args->exprs[0])) {
                if (const auto* str = std::get_if<StringRef>(&literal->value)) {
                    return *str;
                }
                fatalError("Attribute "_t + key + " must be a string literal", false);
            }
        }
    }
    return "";
}

bool AstAttributeList::exists(StringRef name) const noexcept {
    auto iter = std::find_if(attribs.begin(), attribs.end(), [&](const auto& attr) {
        return attr->identExpr->name == name;
    });
    return iter != attribs.end();
}
