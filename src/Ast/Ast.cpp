//
// Created by Albert Varaksin on 05/07/2020.
//
#include "Ast.h"
#include "AstVisitor.h"
#include "Type/Type.h"
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

std::optional<StringRef> AstAttributeList::getStringLiteral(StringRef key) const {
    for (const auto& attr : attribs) {
        if (attr->identExpr->name == key) {
            if (attr->argExprs.size() != 1) {
                fatalError("Attribute "_t + key + " must have 1 value", false);
            }
            if (auto* literal = dyn_cast<AstLiteralExpr>(attr->argExprs[0].get())) {
                if (const auto* str = std::get_if<StringRef>(&literal->value)) {
                    return *str;
                }
                fatalError("Attribute "_t + key + " must be a string literal", false);
            }
        }
    }
    return "";
}

bool AstLiteralExpr::isNegative() const noexcept {
    if (type == nullptr) {
        return false;
    }

    if (const auto* integral = dyn_cast<TypeIntegral>(type)) {
        if (!integral->isSigned()) {
            return false;
        }
        return static_cast<int64_t>(std::get<uint64_t>(value)) < 0;
    }

    if (const auto* fp = dyn_cast<TypeFloatingPoint>(type)) {
        return std::get<double>(value) < 0.0;
    }

    return false;
}
