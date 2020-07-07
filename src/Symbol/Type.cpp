//
// Created by Albert on 07/07/2020.
//
#include "Type.h"
using namespace lbc;
using boost::hash_combine;

namespace {

size_t voidHash() {
    return std::hash<TypeKind>{}(TypeKind::Void);
}

size_t anyHash() {
    return std::hash<TypeKind>{}(TypeKind::Any);
}

size_t pointerHash(const TypeRoot* base) {
    size_t seed = 0;
    hash_combine(seed, std::hash<TypeKind>{}(TypeKind::Pointer));
    hash_combine(seed, base->hash());
    return seed;
}

size_t boolHash() {
    size_t seed = 0;
    hash_combine(seed, std::hash<TypeKind>{}(TypeKind::Number));
    hash_combine(seed, std::hash<TypeKind>{}(TypeKind::Bool));
    return seed;
}

size_t integerHash(int bits, bool isSigned) {
    size_t seed = 0;
    hash_combine(seed, std::hash<TypeKind>{}(TypeKind::Number));
    hash_combine(seed, std::hash<TypeKind>{}(TypeKind::Integer));
    hash_combine(seed, bits);
    hash_combine(seed, isSigned);
    return seed;
}

size_t floatingPointHash(int bits) {
    size_t seed = 0;
    hash_combine(seed, std::hash<TypeKind>{}(TypeKind::Number));
    hash_combine(seed, std::hash<TypeKind>{}(TypeKind::FloatingPoint));
    hash_combine(seed, bits);
    return seed;
}

size_t functionHash(const TypeRoot* retTy, const vector<const TypeRoot*>& params) {
    size_t seed = 0;
    hash_combine(seed, std::hash<TypeKind>{}(TypeKind::Function));
    hash_combine(seed, retTy->hash());
    for (auto param: params) {
        hash_combine(seed, param->hash());
    }
    return seed;
}

size_t zstringHash() {
    return std::hash<TypeKind>{}(TypeKind::ZString);
}

// Declared declaredTypes.
std::unordered_map<size_t, unique_ptr<TypeRoot>> declaredTypes;

// Commonly used types
const TypeVoid    voidTy   { voidHash() };                          // VOID
const TypeAny     anyTy    { anyHash() };                           // Any type
const TypePointer anyPtrTy { pointerHash(&anyTy), &anyTy };         // Any Ptr / void*
const TypeBool    boolTy   { boolHash() };                          // bool
const TypeInteger intTy    { integerHash(32, true), 32, true };     // int
const TypeInteger uintTy   { integerHash(32, false), 32, false };   // unsigned
const TypeZString zstrTy   { zstringHash() };                       // zstring / char *

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
    if (base->hash() == anyTy.hash()) {
        return &anyPtrTy;
    }

    auto hash = pointerHash(base);
    auto iter = declaredTypes.find(hash);
    if (iter != declaredTypes.end()) {
        return dyn_cast<TypePointer>(iter->second.get());
    }

    auto ty = new TypePointer{hash, base};
    declaredTypes.emplace(hash, ty);
    return ty;
}

llvm::Type *TypePointer::llvm() {
    return nullptr;
}

// Bool

const TypeBool *TypeBool::get() {
    return &boolTy;
}

llvm::Type *TypeBool::llvm() {
    return nullptr;
}

// Integer

const TypeInteger *TypeInteger::get(int bits, bool isSigned) {
    if (bits == 32) {
        if (isSigned) return &intTy;
        return &uintTy;
    }

    auto hash = integerHash(bits, isSigned);
    auto iter = declaredTypes.find(hash);
    if (iter != declaredTypes.end()) {
        return dyn_cast<TypeInteger>(iter->second.get());
    }

    auto ty = new TypeInteger{hash, bits, isSigned};
    declaredTypes.emplace(hash, ty);
    return ty;
}

llvm::Type *TypeInteger::llvm() {
    return nullptr;
}

// Floating Point

const TypeFloatingPoint *TypeFloatingPoint::get(int bits) {
    auto hash = floatingPointHash(bits);
    auto iter = declaredTypes.find(hash);
    if (iter != declaredTypes.end()) {
        return dyn_cast<TypeFloatingPoint>(iter->second.get());
    }

    auto ty = new TypeFloatingPoint{hash, bits};
    declaredTypes.emplace(hash, ty);
    return ty;
}

llvm::Type *TypeFloatingPoint::llvm() {
    return nullptr;
}

// Function

const TypeFunction *TypeFunction::get(const TypeRoot* retType, vector<const TypeRoot*>&& paramTypes) {
    auto hash = functionHash(retType, paramTypes);
    auto iter = declaredTypes.find(hash);
    if (iter != declaredTypes.end()) {
        return dyn_cast<TypeFunction>(iter->second.get());
    }

    auto ty = new TypeFunction{hash, retType, std::move(paramTypes)};
    declaredTypes.emplace(hash, ty);
    return ty;
}

llvm::Type *TypeFunction::llvm() {
    return nullptr;
}

// ZString
const TypeZString *TypeZString::get() {
    return &zstrTy;
}

llvm::Type *TypeZString::llvm() {
    return nullptr;
}
