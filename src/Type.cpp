//
//  Type.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 29/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "Type.h"
#include "Token.h"
#include "Context.h"

using namespace lbc;

//static std::unordered_map<TokenType,     Type * > _primitives;
static std::unordered_map<Type *,        std::unordered_map<int, PtrType *>> _ptrTypes;

static PrimitiveType _primitives[] = {
    #define P(ID, STR, SIZE, F) \
        PrimitiveType((Type::TypeKind)(F | Type::Primitive), SIZE),
    PRIMITIVE_TYPES(P)
    #undef P
};



/**
 * Create type
 */
Type::Type(Type *base, TypeKind kind, bool instantiable)
:   m_baseType(base),
    m_kind((TypeKind)(kind | (instantiable ? TypeKind::Instantiable : 0))),
    m_llvm(nullptr)
{
    
}


/**
 * cleanup
 */
Type::~Type() {}


/**
 * Compare this to another type
 */
bool Type::compare(Type *type) const
{
    if (this == type) return true;
    if (kind() != type->kind()) return false;
    
    return this->equal(type);
}


//----------------------------------------------------------------------------------------------------------------------
// Primitive Type
//----------------------------------------------------------------------------------------------------------------------


/**
 * create basic type
 */
PrimitiveType::PrimitiveType(TypeKind kind, unsigned size)
: Type(nullptr, kind, true), m_size(size)
{}


/**
 * clean up
 */
PrimitiveType::~PrimitiveType() {}


/**
 * get basic type
 */
Type * PrimitiveType::get(TokenType type)
{    
    auto idx = (int)type - (int)TokenType::Byte;
    assert(idx >= 0 && idx < (int)sizeof(_primitives) && "Invalid index. Something wrong with the tokentype");
    return &_primitives[idx];
}


/**
 * is this type equal to the give type?
 */
bool PrimitiveType::equal(Type *type) const
{
    return getSizeInBits() == type->getSizeInBits();
}


/**
 * get type as string
 */
std::string PrimitiveType::toString()
{
    for (size_t i = 0; i < sizeof(_primitives); i++) {
        if (&_primitives[i] == this) {
            TokenType idx = (TokenType)(i + (int)TokenType::Byte);
            #define F(ID, STR, ...) if (idx == TokenType::ID) return STR;
            PRIMITIVE_TYPES(F)
            #undef F
        }
    }
    return "Invalid-Type";
}


/**
 * Return llvm::Type representation of the internal type
 */
llvm::Type * PrimitiveType::genLlvmType()
{
    if (isIntegral()) {
        return llvm::Type::getIntNTy(llvm::getGlobalContext(), getSizeInBits());
    } else if (isFloatingPoint()) {
        if (getSizeInBits() == 32)
            return llvm::Type::getFloatTy(llvm::getGlobalContext());
        else if (getSizeInBits() == 64)
            return llvm::Type::getDoubleTy(llvm::getGlobalContext());
        else if (getSizeInBits() == 80)
            return llvm::Type::getX86_FP80Ty(llvm::getGlobalContext());
    }
    return nullptr;
}


//----------------------------------------------------------------------------------------------------------------------
// Pointer Type
//----------------------------------------------------------------------------------------------------------------------


/**
 * get shared ptr type instance
 */
PtrType * PtrType::get(Type *  base, int indirection)
{
    // return ptr wih maximum indirection and least deref level
    if (base->isPointer()) {
        return get(base->getBaseType(), indirection + static_cast<PtrType *>(base)->indirection());
    }
    
    // retrun pointer to non pointer
    auto & inner = _ptrTypes[base];
    auto iter = inner.find(indirection);
    if (iter != inner.end()) return iter->second;

    auto ptr = new PtrType(base, indirection);
    inner[indirection] = ptr;
    return ptr;
}


/**
 * get Any ptr
 */
PtrType * PtrType::getAnyPtr()
{
    static PtrType type(PrimitiveType::get(TokenType::UByte), 1, TypeKind::Pointer | TypeKind::AnyPtr);
    return &type;
}


/**
 * clean up
 */
PtrType::~PtrType() {}


/**
 * Get pointer size
 */
unsigned PtrType::getSizeInBits() const
{
    auto & triple = Context::getGlobalContext().triple();
    if (triple.isArch64Bit()) return 64;
    else if (triple.isArch32Bit()) return 32;
    else if (triple.isArch16Bit()) return 16;
    return 0;
}


/**
 * create ptr type
 */
PtrType::PtrType(Type *base, int level, int flags)
: Type(base, (TypeKind)flags, true), m_level(level)
{}


/**
 * compare ptr type to another type
 */
bool PtrType::equal(Type *type) const
{
    // cast
    auto other = static_cast<PtrType *>(type);
    return m_level == other->m_level && getBaseType()->compare(other->getBaseType());
}


/**
 * to string
 */
std::string PtrType::toString()
{
    std::string result = getBaseType()->toString();
    for (int i = 0; i < indirection(); i++)
        result += " ptr";
    return result;
}


/**
 * dereference to hold pointer
 */
Type * PtrType::dereference(int levels) const
{
    assert(levels > 0);
    assert(levels <= indirection());
    
    int n = indirection() - levels;
    if (n == 0) {
        return getBaseType();
    } else {
        return PtrType::get(getBaseType(), n);
    }
}


/**
 * Return llvm::Type representation of the internal type
 */
llvm::Type * PtrType::genLlvmType()
{
    if (getBaseType()->isPointer()) {
        THROW_EXCEPTION("IS pointer");
    }
    auto type = getBaseType()->llvm()->getPointerTo();
    int level = indirection() - 1;
    while(level--) {
        type = type->getPointerTo();
    }
    return type;
}


//----------------------------------------------------------------------------------------------------------------------
// Function Type
//----------------------------------------------------------------------------------------------------------------------


/**
 * create function type
 */
FunctionType::FunctionType(Type *result, bool va)
: Type(result, TypeKind::Function, false), vararg(va)
{}


/**
 * Get size of the function type (size of the pointer)
 */
unsigned FunctionType::getSizeInBits() const
{
    auto & triple = Context::getGlobalContext().triple();
    if (triple.isArch64Bit()) return 64;
    else if (triple.isArch32Bit()) return 32;
    else if (triple.isArch16Bit()) return 16;
    return 0;
}



/**
 * Compare two types
 */
static bool compare_types_pred(Type *typ1, Type *typ2)
{
    return typ1->compare(typ2);
}



/**
 * compare function type to another type
 */
bool FunctionType::equal(Type *type) const
{
    // cast
    auto other = static_cast<FunctionType *>(type);
    
    // differnt number of arguments?
    if (params.size() != other->params.size()) return false;
    
    // check return type
    if (!getBaseType()->compare(other->getBaseType())) return false;
    
    // Compare the parameters
    return std::equal(params.begin(), params.end(), other->params.begin(), compare_types_pred);
}


/**
 * clean up
 */
FunctionType::~FunctionType() {}


/**
 * to string
 */
std::string FunctionType::toString()
{
    std::string result =  "FUNCTION (";
    bool first = true;
    for(auto & t : params) {
        if (first) first = false;
        else result += ", ";
        result += t->toString();
    }
    return result + ") AS " + this->result()->toString();
}


/**
 * Return llvm::Type representation of the internal type
 */
llvm::Type * FunctionType::genLlvmType()
{
    std::vector<llvm::Type*> pv;
    for (auto p : params) {
        pv.push_back(p->llvm());
    }
    llvm::Type * res = result() ? result()->llvm() : llvm::Type::getVoidTy(llvm::getGlobalContext());
    return llvm::FunctionType::get(res, pv, vararg);
}







