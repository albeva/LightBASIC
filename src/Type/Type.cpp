//
// Created by Albert Varaksin on 07/07/2020.
//
#include "Type.h"
#include "Driver/Context.h"
#include "Lexer/Token.h"
using namespace lbc;

namespace {

// Declared declaredTypes.
static std::vector<unique_ptr<TypeFunction>> declaredFunc; // NOLINT
static std::vector<unique_ptr<TypePointer>> declaredPtrs;  // NOLINT

// Commonly used types
const TypeVoid voidTy{};              // VOID
const TypeAny anyTy{};                // Any typeExpr
const TypePointer anyPtrTy{ &anyTy }; // void*

// primitives
#define DEFINE_TYPE(id, str, kind) \
    const Type##kind id##Ty;
PRIMITIVE_TYPES(DEFINE_TYPE)
#undef DEFINE_TYPE

// integers
#define DEFINE_TYPE(id, str, kind, bits, isSigned, ...) \
    const Type##kind id##Ty{ bits, isSigned };
INTEGRAL_TYPES(DEFINE_TYPE)
#undef DEFINE_TYPE

// Floating Points
#define DEFINE_TYPE(id, str, kind, bits, ...) \
    const Type##kind id##Ty{ bits };
FLOATINGPOINT_TYPES(DEFINE_TYPE)
#undef DEFINE_TYPE

} // namespace

const TypeRoot* TypeRoot::fromTokenKind(TokenKind kind) noexcept {
#define CASE_PRIMITIVE(id, str, KIND, ...) \
    case TokenKind::id:                    \
        return Type##KIND::get();
#define CASE_INTEGER(id, str, KIND, bits, isSigned, ...) \
    case TokenKind::id:                                  \
        return Type##KIND::get(bits, isSigned);
#define CASE_FLOATINGPOINT(id, str, KIND, bits, ...) \
    case TokenKind::id:                              \
        return Type##KIND::get(bits);

    switch (kind) {
        PRIMITIVE_TYPES(CASE_PRIMITIVE)
        INTEGRAL_TYPES(CASE_INTEGER)
        FLOATINGPOINT_TYPES(CASE_FLOATINGPOINT)
    default:
        fatalError("Unknown typeExpr "_t + Token::description(kind), false);
    }

#undef TO_PRIMITIVE_TYPE
#undef CASE_INTEGER
#undef CASE_INTEGER
}

bool TypeRoot::isAnyPointer() const noexcept {
    return this == &anyPtrTy;
}

bool TypeRoot::isSignedIntegral() const noexcept {
    if (!isIntegral()) {
        return false;
    }
    return static_cast<const TypeIntegral*>(this)->isSigned(); // NOLINT
}

// clang-format off
#define CHECK_TYPE_IMPL(ID, ...)             \
    bool TypeRoot::is##ID() const noexcept { \
        return this == &ID##Ty;              \
    }
    INTEGRAL_TYPES(CHECK_TYPE_IMPL)
    FLOATINGPOINT_TYPES(CHECK_TYPE_IMPL)
#undef CHECK_TYPE_IMPL
// clang-format on

TypeComparison TypeRoot::compare(const TypeRoot *other) const noexcept {
    if (this == other) {
        return TypeComparison::Equal;
    }

    if (const auto* left = dyn_cast<TypeIntegral>(this)) {
        if (const auto* right = dyn_cast<TypeIntegral>(other)) {
            if (left->getBits() > right->getBits()) {
                return TypeComparison::Downcast;
            }
            if (left->getBits() < right->getBits()) {
                return TypeComparison::Upcast;
            }
            if (left->isSigned()) {
                return TypeComparison::Downcast;
            }
            if (right->isSigned()) {
                return TypeComparison::Upcast;
            }
        } else if (other->isFloatingPoint()) {
            return TypeComparison::Upcast;
        }
        return TypeComparison::Incompatible;
    }

    if (const auto& left = dyn_cast<TypeFloatingPoint>(this)) {
        if (other->isIntegral()) {
            return TypeComparison::Downcast;
        }
        if (const auto* right = dyn_cast<TypeFloatingPoint>(other)) {
            if (left->getBits() > right->getBits()) {
                return TypeComparison::Downcast;
            }
            return TypeComparison::Upcast;
        }
    }

    return TypeComparison::Incompatible;
}

// Void

const TypeVoid* TypeVoid::get() noexcept {
    return &voidTy;
}

llvm::Type* TypeVoid::genLlvmType(Context& context) const noexcept {
    return llvm::Type::getVoidTy(context.getLlvmContext());
}

string TypeVoid::asString() const noexcept {
    return "VOID";
}

// Any
const TypeAny* TypeAny::get() noexcept {
    return &anyTy;
}

llvm::Type* TypeAny::genLlvmType(Context& context) const noexcept {
    return llvm::Type::getInt8Ty(context.getLlvmContext());
}

string TypeAny::asString() const noexcept {
    return "ANY";
}

// Pointer

const TypePointer* TypePointer::get(const TypeRoot* base) noexcept {
    if (base == &anyTy) {
        return &anyPtrTy;
    }

    for (const auto& ptr : declaredPtrs) {
        if (ptr->m_base == base) {
            return ptr.get();
        }
    }

    return declaredPtrs.emplace_back(make_unique<TypePointer>(base)).get();
}

llvm::Type* TypePointer::genLlvmType(Context& context) const noexcept {
    return llvm::PointerType::get(m_base->getLlvmType(context), 0);
}

string TypePointer::asString() const noexcept {
    return m_base->asString() + " PTR";
}

// Bool

const TypeBoolean* TypeBoolean::get() noexcept {
    return &BoolTy;
}

llvm::Type* TypeBoolean::genLlvmType(Context& context) const noexcept {
    return llvm::Type::getInt1Ty(context.getLlvmContext());
}

string TypeBoolean::asString() const noexcept {
    return "BOOL";
}

// Integer

const TypeIntegral* TypeIntegral::get(unsigned bits, bool isSigned) noexcept {
#define USE_TYPE(id, str, kind, BITS, IS_SIGNED, ...) \
    if (bits == BITS && isSigned == IS_SIGNED)        \
        return &id##Ty;
    INTEGRAL_TYPES(USE_TYPE)
#undef USE_TYPE

    fatalError("Invalid integer type size: "_t + Twine(bits), false);
}

llvm::Type* TypeIntegral::genLlvmType(Context& context) const noexcept {
    return llvm::IntegerType::get(context.getLlvmContext(), getBits());
}

string TypeIntegral::asString() const noexcept {
#define GET_TYPE(id, str, kind, BITS, SIGNED, ...) \
    if (getBits() == BITS && isSigned() == SIGNED) \
        return str;
    INTEGRAL_TYPES(GET_TYPE)
#undef GET_TYPE

    llvm_unreachable("unknown integer type");
}

// Floating Point

const TypeFloatingPoint* TypeFloatingPoint::get(unsigned bits) noexcept {
    switch (bits) {
#define USE_TYPE(id, str, kind, BITS, ...) \
    case BITS:                             \
        return &id##Ty;
        FLOATINGPOINT_TYPES(USE_TYPE)
#undef USE_TYPE
    default:
        fatalError("Invalid floating point type size: "_t + Twine(bits), false);
    }
}

llvm::Type* TypeFloatingPoint::genLlvmType(Context& context) const noexcept {
    switch (getBits()) {
    case 32: // NOLINT
        return llvm::Type::getFloatTy(context.getLlvmContext());
    case 64: // NOLINT
        return llvm::Type::getDoubleTy(context.getLlvmContext());
    default:
        fatalError("Invalid floating point type size: "_t + Twine(getBits()), false);
    }
}

string TypeFloatingPoint::asString() const noexcept {
#define GET_TYPE(id, str, kind, BITS, ...) \
    if (getBits() == BITS)                 \
        return str;
    FLOATINGPOINT_TYPES(GET_TYPE)
#undef GET_TYPE

    llvm_unreachable("unknown floating point type");
}

// Function

const TypeFunction* TypeFunction::get(const TypeRoot* retType, std::vector<const TypeRoot*>&& paramTypes, bool variadic) noexcept {
    for (const auto& ptr : declaredFunc) {
        if (ptr->getReturn() == retType && ptr->getParams() == paramTypes && ptr->isVariadic() == variadic) {
            return ptr.get();
        }
    }

    auto ty = make_unique<TypeFunction>(retType, std::move(paramTypes), variadic);
    return declaredFunc.emplace_back(std::move(ty)).get();
}

llvm::Type* TypeFunction::genLlvmType(Context& context) const noexcept {
    auto* retTy = m_retType->getLlvmType(context);

    std::vector<llvm::Type*> params;
    params.reserve(m_paramTypes.size());
    for (const auto& p : m_paramTypes) {
        params.emplace_back(p->getLlvmType(context));
    }

    return llvm::FunctionType::get(retTy, params, m_variadic);
}

string TypeFunction::asString() const noexcept {
    string out = m_retType != nullptr ? "FUNCTION(" : "SUB(";
    for (size_t i = 0; i < m_paramTypes.size(); i++) {
        if (i > 0) {
            out += ", ";
        }
        out += m_paramTypes[i]->asString();
    }
    out += ")";
    if (m_retType != nullptr) {
        out += " AS " + m_retType->asString();
    }
    return out;
}

// ZString
const TypeZString* TypeZString::get() noexcept {
    return &ZStringTy;
}

llvm::Type* TypeZString::genLlvmType(Context& context) const noexcept {
    return llvm::Type::getInt8PtrTy(context.getLlvmContext());
}

string TypeZString::asString() const noexcept {
    return "ZSTRING";
}
