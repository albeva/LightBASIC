//
// Created by Albert on 07/07/2020.
//
#include "Type.h"
using namespace lbc;

namespace {

// Declared declaredTypes.
std::vector<unique_ptr<const TypeFunction>> declaredFunc;
std::vector<unique_ptr<const TypePointer>> declaredPtrs;
std::vector<unique_ptr<const TypeInteger>> declaredInts;
std::vector<unique_ptr<const TypeFloatingPoint>> declaredFPs;

// Commonly used types
const TypeVoid          voidTy;             // VOID
const TypeAny           anyTy;              // Any type
const TypePointer       anyPtrTy{ &anyTy }; // void*

// primitives
#define DEFINE_TYPE(id, str, kind) \
    const Type##kind id##Ty;
PRIMITIVE_TYPES(DEFINE_TYPE)
#undef DEFINE_TYPE

// integers
#define DEFINE_TYPE(id, str, bits, isSigned, kind) \
    const Type##kind id##Ty{bits, isSigned};
INTEGER_TYPES(DEFINE_TYPE)
#undef DEFINE_TYPE

// Floating Points
#define DEFINE_TYPE(id, str, bits, kind) \
    const Type##kind id##Ty{bits};
FLOATINGPOINT_TYPES(DEFINE_TYPE)
#undef DEFINE_TYPE

} // namespace

TypeRoot::~TypeRoot() {}

// Void

const TypeVoid *TypeVoid::get() {
    return &voidTy;
}

llvm::Type *TypeVoid::llvm() {
    return nullptr;
}

// Any
const TypeAny *TypeAny::get() {
    return &anyTy;
}

llvm::Type *TypeAny::llvm() {
    return nullptr;
}

// Pointer

const TypePointer *TypePointer::get(const TypeRoot *base) {
    if (base == &anyTy) {
        return &anyPtrTy;
    }

    for (const auto& ptr: declaredPtrs) {
        if (ptr->m_base == base)
            return ptr.get();
    }

    return declaredPtrs.emplace_back(make_unique<TypePointer>(base)).get();
}

llvm::Type *TypePointer::llvm() {
    return nullptr;
}

// Bool

const TypeBool *TypeBool::get() {
    return &BoolTy;
}

llvm::Type *TypeBool::llvm() {
    return nullptr;
}

// Integer

const TypeInteger *TypeInteger::get(int bits, bool isSigned) {
    #define USE_TYPE(id, str, BITS, IS_SIGNED, kind) \
        if (bits == BITS && isSigned == IS_SIGNED) return &id##Ty;
        INTEGER_TYPES(USE_TYPE)
    #undef USE_TYPE

    for (const auto& ptr: declaredInts) {
        if (ptr->bits() == bits && ptr->isSigned() == isSigned)
            return ptr.get();
    }

    return declaredInts.emplace_back(make_unique<TypeInteger>(bits, isSigned)).get();
}

llvm::Type *TypeInteger::llvm() {
    return nullptr;
}

// Floating Point

const TypeFloatingPoint *TypeFloatingPoint::get(int bits) {
    #define USE_TYPE(id, str, BITS, kind) \
        if (bits == BITS) return &id##Ty;
        FLOATINGPOINT_TYPES(USE_TYPE)
    #undef USE_TYPE

    for (const auto& ptr: declaredFPs) {
        if (ptr->bits() == bits)
            return ptr.get();
    }

    return declaredFPs.emplace_back(make_unique<TypeFloatingPoint>(bits)).get();
}

llvm::Type *TypeFloatingPoint::llvm() {
    return nullptr;
}

// Function

const TypeFunction *TypeFunction::get(const TypeRoot* retType, vector<const TypeRoot*>&& paramTypes) {
    for (const auto& ptr: declaredFunc) {
        if (ptr->retType() == retType && ptr->paramTypes() == paramTypes)
            return ptr.get();
    }

    return declaredFunc.emplace_back(make_unique<TypeFunction>(
        retType,
        std::move(paramTypes)
    )).get();
}

llvm::Type *TypeFunction::llvm() {
    return nullptr;
}

// ZString
const TypeZString *TypeZString::get() {
    return &ZStrringTy;
}

llvm::Type *TypeZString::llvm() {
    return nullptr;
}
