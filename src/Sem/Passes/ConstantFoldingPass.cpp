//
// Created by Albert Varaksin on 05/05/2021.
//
#include "ConstantFoldingPass.h"
#include "Type/Type.h"
using namespace lbc;
using namespace Sem;

namespace {
template<typename BASE, typename T>
inline BASE castLiteral(AstLiteralExpr* ast) noexcept {
    constexpr auto visitor = [](const auto& val) -> T {
        using R = std::decay_t<decltype(val)>;
        if constexpr (std::is_convertible_v<R, T>) {
            return static_cast<T>(val);
        } else {
            llvm_unreachable("Unsupported type conversion");
        }
    };
    return static_cast<BASE>(std::visit(visitor, ast->value));
}
} // namespace

void ConstantFoldingPass::fold(unique_ptr<AstExpr>& ast) noexcept {
    unique_ptr<AstExpr> replace;

    switch (ast->kind()) {
    case AstKind::UnaryExpr:
        replace = visitUnaryExpr(static_cast<AstUnaryExpr*>(ast.get())); // NOLINT
        break;
    case AstKind::CastExpr:
        replace = visitCastExpr(static_cast<AstCastExpr*>(ast.get())); // NOLINT
        break;
    default:
        return;
    }

    if (replace != nullptr) {
        ast.swap(replace);
    }
}

unique_ptr<AstExpr> ConstantFoldingPass::visitUnaryExpr(AstUnaryExpr* ast) noexcept {
    auto* literal = dyn_cast<AstLiteralExpr>(ast->expr.get());
    if (literal == nullptr) {
        return nullptr;
    }

    auto replacement = AstLiteralExpr::create();
    replacement->type = literal->type;

    if (ast->tokenKind == TokenKind::Negate) {
        constexpr auto visitor = Visitor{
            [](uint64_t integral) -> AstLiteralExpr::Value {
                return static_cast<uint64_t>(-static_cast<int64_t>(integral));
            },
            [](double fp) -> AstLiteralExpr::Value {
                return -fp;
            },
            [](auto) -> AstLiteralExpr::Value {
                llvm_unreachable("Non supported type");
            }
        };
        replacement->value = std::visit(visitor, literal->value);
    } else {
        llvm_unreachable("Unsupported unary operation");
    }

    return replacement;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitCastExpr(AstCastExpr* ast) noexcept {
    auto* literal = dyn_cast<AstLiteralExpr>(ast->expr.get());
    if (literal == nullptr) {
        return nullptr;
    }

    auto replacement = AstLiteralExpr::create();
    replacement->type = ast->type;

    // clang-format off
    if (const auto* integral = dyn_cast<TypeIntegral>(ast->type)) {
        #define INTEGRAL(ID, STR, KIND, BITS, SIGNED, TYPE)                      \
            if (integral->getBits() == BITS && integral->isSigned() == SIGNED) { \
                replacement->value = castLiteral<uint64_t, TYPE>(literal);       \
                return replacement;                                              \
            }
        INTEGRAL_TYPES(INTEGRAL)
        #undef INTEGRAL
    } else if (const auto* fp = dyn_cast<TypeFloatingPoint>(ast->type)) {
        #define FLOATINGPOINT(ID, STR, KIND, BITS, TYPE)                 \
            if (fp->getBits() == BITS) {                                 \
                replacement->value = castLiteral<double, TYPE>(literal); \
                return replacement;                                      \
            }
        FLOATINGPOINT_TYPES(FLOATINGPOINT)
        #undef INTEGRAL
    }
    // clang-format on
    llvm_unreachable("Unsupported castLiteral");
}
