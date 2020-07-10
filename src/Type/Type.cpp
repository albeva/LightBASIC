//
// Created by Albert on 07/07/2020.
//
#include "Type.h"
#include "Lexer/Token.h"
using namespace lbc;

namespace {

// Declared declaredTypes.
std::vector<unique_ptr<const TypeFunction>> declaredFunc;
std::vector<unique_ptr<const TypePointer>> declaredPtrs;
std::vector<unique_ptr<const TypeInteger>> declaredInts;
std::vector<unique_ptr<const TypeFloatingPoint>> declaredFPs;

// Commonly used types
const TypeVoid voidTy;                // VOID
const TypeAny anyTy;                  // Any typeExpr
const TypePointer anyPtrTy{ &anyTy }; // void*

// primitives
#define DEFINE_TYPE(id, str, kind) \
    const Type##kind id##Ty;
PRIMITIVE_TYPES(DEFINE_TYPE)
#undef DEFINE_TYPE

// integers
#define DEFINE_TYPE(id, str, kind, bits, isSigned) \
    const Type##kind id##Ty{ bits, isSigned };
INTEGER_TYPES(DEFINE_TYPE)
#undef DEFINE_TYPE

// Floating Points
#define DEFINE_TYPE(id, str, kind, bits) \
    const Type##kind id##Ty{ bits };
FLOATINGPOINT_TYPES(DEFINE_TYPE)
#undef DEFINE_TYPE

} // namespace

const TypeRoot* TypeRoot::fromTokenKind(TokenKind kind) {
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
        INTEGER_TYPES(CASE_INTEGER)
        FLOATINGPOINT_TYPES(CASE_FLOATINGPOINT)
    default:
        std::cerr << "Unknown typeExpr "s + string(Token::description(kind));
        std::exit(EXIT_FAILURE);
    }

#undef TO_PRIMITIVE_TYPE
#undef CASE_INTEGER
#undef CASE_INTEGER
}

// Void

const TypeVoid* TypeVoid::get() {
    return &voidTy;
}

llvm::Type* TypeVoid::genLlvmType() const {
    return nullptr;
}

// Any
const TypeAny* TypeAny::get() {
    return &anyTy;
}

llvm::Type* TypeAny::genLlvmType() const {
    return nullptr;
}

// Pointer

const TypePointer* TypePointer::get(const TypeRoot* base) {
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

llvm::Type* TypePointer::genLlvmType() const {
    return nullptr;
}

// Bool

const TypeBool* TypeBool::get() {
    return &BoolTy;
}

llvm::Type* TypeBool::genLlvmType() const {
    return nullptr;
}

// Integer

const TypeInteger* TypeInteger::get(int bits, bool isSigned) {
#define USE_TYPE(id, str, kind, BITS, IS_SIGNED) \
    if (bits == BITS && isSigned == IS_SIGNED)   \
        return &id##Ty;
    INTEGER_TYPES(USE_TYPE)
#undef USE_TYPE

    for (const auto& ptr : declaredInts) {
        if (ptr->bits() == bits && ptr->isSigned() == isSigned) {
            return ptr.get();
        }
    }

    return declaredInts.emplace_back(make_unique<TypeInteger>(bits, isSigned)).get();
}

llvm::Type* TypeInteger::genLlvmType() const {
    return nullptr;
}

// Floating Point

const TypeFloatingPoint* TypeFloatingPoint::get(int bits) {
#define USE_TYPE(id, str, kind, BITS) \
    if (bits == BITS) {               \
        return &id##Ty;               \
    }
    FLOATINGPOINT_TYPES(USE_TYPE)
#undef USE_TYPE

    for (const auto& ptr : declaredFPs) {
        if (ptr->bits() == bits) {
            return ptr.get();
        }
    }

    return declaredFPs.emplace_back(make_unique<TypeFloatingPoint>(bits)).get();
}

llvm::Type* TypeFloatingPoint::genLlvmType() const {
    return nullptr;
}

// Function

const TypeFunction* TypeFunction::get(const TypeRoot* retType, std::vector<const TypeRoot*>&& paramTypes) {
    for (const auto& ptr : declaredFunc) {
        if (ptr->retType() == retType && ptr->paramTypes() == paramTypes) {
            return ptr.get();
        }
    }

    auto ty = make_unique<TypeFunction>(retType, std::move(paramTypes));
    return declaredFunc.emplace_back(std::move(ty)).get();
}

llvm::Type* TypeFunction::genLlvmType() const {
    return nullptr;
}

// ZString
const TypeZString* TypeZString::get() {
    return &ZStringTy;
}

llvm::Type* TypeZString::genLlvmType() const {
    return nullptr;
}
