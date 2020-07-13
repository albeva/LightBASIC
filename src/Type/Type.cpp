//
// Created by Albert on 07/07/2020.
//
#include "Type.h"
#include "Lexer/Token.h"
using namespace lbc;

namespace {

// Declared declaredTypes.
static std::vector<unique_ptr<TypeFunction>> declaredFunc;
static std::vector<unique_ptr<TypePointer>> declaredPtrs;

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

TypeRoot::~TypeRoot() = default;

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

llvm::Type* TypeVoid::genLlvmType(llvm::LLVMContext& context) const {
    // should never be called?
    return llvm::Type::getVoidTy(context);
}

// Any
const TypeAny* TypeAny::get() {
    return &anyTy;
}

llvm::Type* TypeAny::genLlvmType(llvm::LLVMContext& context) const {
    return llvm::Type::getInt8Ty(context);
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

llvm::Type* TypePointer::genLlvmType(llvm::LLVMContext& context) const {
    return llvm::PointerType::get(m_base->llvmType(context), 0);
}

// numeric base
TypeNumber::~TypeNumber() = default;

// Bool

const TypeBool* TypeBool::get() {
    return &BoolTy;
}

llvm::Type* TypeBool::genLlvmType(llvm::LLVMContext& context) const {
    return llvm::Type::getInt1Ty(context);
}

// Integer

const TypeInteger* TypeInteger::get(unsigned bits, bool isSigned) {
#define USE_TYPE(id, str, kind, BITS, IS_SIGNED) \
    if (bits == BITS && isSigned == IS_SIGNED)   \
        return &id##Ty;
    INTEGER_TYPES(USE_TYPE)
#undef USE_TYPE

    std::cerr << "Invalid integer type size: " << bits << std::endl;
    std::exit(EXIT_FAILURE);
}

llvm::Type* TypeInteger::genLlvmType(llvm::LLVMContext& context) const {
    return llvm::IntegerType::get(context, bits());
}

// Floating Point

const TypeFloatingPoint* TypeFloatingPoint::get(unsigned bits) {
    switch (bits) {
#define USE_TYPE(id, str, kind, BITS) \
    case BITS:                        \
        return &id##Ty;
        FLOATINGPOINT_TYPES(USE_TYPE)
#undef USE_TYPE
    default:
        std::cerr << "Invalid floating point type size: " << bits << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

llvm::Type* TypeFloatingPoint::genLlvmType(llvm::LLVMContext& context) const {
    switch (bits()) {
    case 32: // NOLINT
        return llvm::Type::getFloatTy(context);
    case 64: // NOLINT
        return llvm::Type::getDoubleTy(context);
    default:
        std::cerr << "Invalid floating point type size: " << bits() << std::endl;
        std::exit(EXIT_FAILURE);
    }
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

llvm::Type* TypeFunction::genLlvmType(llvm::LLVMContext& context) const {
    auto* retTy = m_retType->llvmType(context);

    std::vector<llvm::Type*> params;
    params.reserve(m_paramTypes.size());
    for (const auto& p: m_paramTypes) {
        params.emplace_back(p->llvmType(context));
    }

    return llvm::FunctionType::get(retTy, params, false);
}

// ZString
const TypeZString* TypeZString::get() {
    return &ZStringTy;
}

llvm::Type* TypeZString::genLlvmType(llvm::LLVMContext& context) const {
    return llvm::Type::getInt8PtrTy(context);
}
