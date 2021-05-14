//
// Created by Albert Varaksin on 05/05/2021.
//
#include "ConstantFoldingPass.h"
#include "Type/Type.h"
#include "Driver/Context.h"
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
    if (m_context.getOptimizationLevel() == Context::OptimizationLevel::O0) {
        return;
    }

    unique_ptr<AstExpr> replace;
    switch (ast->kind()) {
    case AstKind::UnaryExpr:
        replace = visitUnaryExpr(static_cast<AstUnaryExpr*>(ast.get())); // NOLINT
        break;
    case AstKind::BinaryExpr:
        replace = visitBinaryExpr(static_cast<AstBinaryExpr*>(ast.get())); // NOLINT
        break;
    case AstKind::CastExpr:
        replace = visitCastExpr(static_cast<AstCastExpr*>(ast.get())); // NOLINT
        break;
    case AstKind::IfExpr:
        replace = visitIfExpr(static_cast<AstIfExpr*>(ast.get())); // NOLINT
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

    auto replacement = AstLiteralExpr::create(ast->getRange());
    replacement->type = literal->type;

    switch (ast->tokenKind) {
    case TokenKind::Negate: {
        constexpr auto visitor = Visitor{
            [](uint64_t value) -> AstLiteralExpr::Value {
                return static_cast<uint64_t>(-static_cast<int64_t>(value));
            },
            [](double value) -> AstLiteralExpr::Value {
                return -value;
            },
            [](auto /*value*/) -> AstLiteralExpr::Value {
                llvm_unreachable("Non supported type");
            }
        };
        replacement->value = std::visit(visitor, literal->value);
        break;
    }
    case TokenKind::LogicalNot: {
        constexpr auto visitor = Visitor{
            [](bool value) -> AstLiteralExpr::Value {
                return !value;
            },
            [](auto /*value*/) -> AstLiteralExpr::Value {
                llvm_unreachable("Non supported type");
            }
        };
        replacement->value = std::visit(visitor, literal->value);
        break;
    }
    default:
        llvm_unreachable("Unsupported unary operation");
    }

    return replacement;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitIfExpr(AstIfExpr* ast) noexcept {
    if (auto* expr = dyn_cast<AstLiteralExpr>(ast->expr.get())) {
        if (std::get<bool>(expr->value)) {
            return std::move(ast->trueExpr);
        }
        return std::move(ast->falseExpr);
    }

    if (auto repl = optimizeIifToCast(ast)) {
        return repl;
    }

    return nullptr;
}

unique_ptr<AstExpr> ConstantFoldingPass::optimizeIifToCast(AstIfExpr* ast) noexcept {
    auto* lhs = dyn_cast<AstLiteralExpr>(ast->trueExpr.get());
    if (lhs == nullptr) {
        return nullptr;
    }
    auto* lval = std::get_if<uint64_t>(&lhs->value);
    if (lval == nullptr) {
        return nullptr;
    }

    auto* rhs = dyn_cast<AstLiteralExpr>(ast->falseExpr.get());
    if (rhs == nullptr) {
        return nullptr;
    }
    auto* rval = std::get_if<uint64_t>(&rhs->value);
    if (rval == nullptr) {
        return nullptr;
    }

    if (*lval == 1 && *rval == 0) {
        auto cast = AstCastExpr::create(ast->getRange());
        cast->expr = std::move(ast->expr);
        cast->type = lhs->type;
        cast->implicit = true;
        return cast;
    }

    if (*lval == 0 && *rval == 1) {
        auto unary = AstUnaryExpr::create(ast->getRange());
        unary->tokenKind = TokenKind::LogicalNot;
        unary->expr = std::move(ast->expr);

        auto cast = AstCastExpr::create(ast->getRange());
        cast->expr = std::move(unary);
        cast->type = lhs->type;
        cast->implicit = true;
        return cast;
    }

    return nullptr;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitBinaryExpr(AstBinaryExpr* /*ast*/) noexcept {
    // TODO
    return nullptr;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitCastExpr(AstCastExpr* ast) noexcept {
    auto* literal = dyn_cast<AstLiteralExpr>(ast->expr.get());
    if (literal == nullptr) {
        return nullptr;
    }

    auto replacement = AstLiteralExpr::create(ast->getRange());
    replacement->type = ast->type;

    // clang-format off
    if (const auto* integral = dyn_cast<TypeIntegral>(ast->type)) {
        #define INTEGRAL(ID, STR, KIND, BITS, SIGNED, TYPE)                          \
            if (integral->getBits() == (BITS) && integral->isSigned() == (SIGNED)) { \
                replacement->value = castLiteral<uint64_t, TYPE>(literal);           \
                return replacement;                                                  \
            }
            INTEGRAL_TYPES(INTEGRAL)
        #undef INTEGRAL
    } else if (const auto* fp = dyn_cast<TypeFloatingPoint>(ast->type)) {
        #define FLOATINGPOINT(ID, STR, KIND, BITS, TYPE)                 \
            if (fp->getBits() == (BITS)) {                               \
                replacement->value = castLiteral<double, TYPE>(literal); \
                return replacement;                                      \
            }
            FLOATINGPOINT_TYPES(FLOATINGPOINT)
        #undef INTEGRAL
    } else if (ast->type->isBoolean()) {
        replacement->value = castLiteral<bool, bool>(literal);
        return replacement;
    }
    // clang-format on

    llvm_unreachable("Unsupported castLiteral");
}
