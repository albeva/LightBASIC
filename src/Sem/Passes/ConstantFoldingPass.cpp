//
// Created by Albert Varaksin on 05/05/2021.
//
#include "ConstantFoldingPass.h"
#include "Driver/Context.h"
#include "Type/Type.h"
using namespace lbc;
using namespace Sem;

namespace {
template<typename BASE, typename T>
constexpr inline BASE castLiteral(const AstLiteralExpr* ast) noexcept {
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
    switch (ast->kind) {
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

unique_ptr<AstExpr> ConstantFoldingPass::visitUnaryExpr(const AstUnaryExpr* ast) noexcept {
    auto* literal = dyn_cast<AstLiteralExpr>(ast->expr.get());
    if (literal == nullptr) {
        return nullptr;
    }

    auto value = unary(ast->tokenKind, literal);
    auto repl = AstLiteralExpr::create(ast->range, value);
    repl->type = ast->type;
    return repl;
}

AstLiteralExpr::Value ConstantFoldingPass::unary(TokenKind op, const AstLiteralExpr* ast) noexcept {
    switch (op) {
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
        return std::visit(visitor, ast->value);
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
        return std::visit(visitor, ast->value);
    }
    default:
        llvm_unreachable("Unsupported unary operation");
    }
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
        auto cast = AstCastExpr::create(
            ast->range,
            std::move(ast->expr),
            nullptr,
            true);
        cast->type = ast->type;
        return cast;
    }

    if (*lval == 0 && *rval == 1) {
        auto unary = AstUnaryExpr::create(
            ast->range,
            TokenKind::LogicalNot,
            std::move(ast->expr));

        auto cast = AstCastExpr::create(
            ast->range,
            std::move(unary),
            nullptr,
            true);
        cast->type = ast->type;
        return cast;
    }

    return nullptr;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitBinaryExpr(AstBinaryExpr* /*ast*/) noexcept {
    // TODO
    return nullptr;
}

unique_ptr<AstExpr> ConstantFoldingPass::visitCastExpr(const AstCastExpr* ast) noexcept {
    auto* literal = dyn_cast<AstLiteralExpr>(ast->expr.get());
    if (literal == nullptr) {
        return nullptr;
    }

    auto value = cast(ast->type, literal);
    auto repl = AstLiteralExpr::create(ast->range, value);
    repl->type = ast->type;
    return repl;
}

AstLiteralExpr::Value ConstantFoldingPass::cast(const TypeRoot* type, const AstLiteralExpr* literal) noexcept {
    // clang-format off
    if (const auto* integral = dyn_cast<TypeIntegral>(type)) {
        #define INTEGRAL(ID, STR, KIND, BITS, SIGNED, TYPE)                          \
            if (integral->getBits() == (BITS) && integral->isSigned() == (SIGNED)) { \
                return castLiteral<uint64_t, TYPE>(literal);                         \
            }
        INTEGRAL_TYPES(INTEGRAL)
        #undef INTEGRAL
    } else if (const auto* fp = dyn_cast<TypeFloatingPoint>(type)) {
        #define FLOATINGPOINT(ID, STR, KIND, BITS, TYPE)   \
            if (fp->getBits() == (BITS)) {                 \
                return castLiteral<double, TYPE>(literal); \
            }
        FLOATINGPOINT_TYPES(FLOATINGPOINT)
        #undef INTEGRAL
    } else if (type->isBoolean()) {
        return castLiteral<bool, bool>(literal);
    } else if (literal->type->isAnyPointer()) {
        return literal->value;
    }
    // clang-format on
    llvm_unreachable("Unsupported castLiteral");
}
