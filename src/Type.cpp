//
//  Type.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 29/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "Type.h"
#include "Token.h"
using namespace lbc;

//static unordered_map<TokenType,     Type * > _primitives;
static unordered_map<Type *,        unordered_map<int, PtrType *>> _ptrTypes;

static PrimitiveType _primitives[] = {
    #define P(ID, STR, SIZE, F, ...) \
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
    llvmType(nullptr)
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
    return this->equal(type);
}



/**
 * create basic type
 */
PrimitiveType::PrimitiveType(TypeKind kind, int size)
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
    assert(idx >= 0 && idx < sizeof(_primitives) && "Invalid index. Something wrong with the tokentype");
    return &_primitives[idx];
}


/**
 * is this type equal to the give type?
 */
bool PrimitiveType::equal(Type *type) const
{
    return false;
    return kind() == type->kind() && kind() == static_cast<PrimitiveType *>(type)->kind();
}



/**
 * get shared ptr type instance
 */
Type * PtrType::get(Type *  base, int indirection)
{
    auto & inner = _ptrTypes[base];
    auto iter = inner.find(indirection);
    if (iter != inner.end()) return iter->second;

    auto ptr = new PtrType(base, indirection);
    inner[indirection] = ptr;
    return ptr;
}


/**
 * clean up
 */
PtrType::~PtrType() {}


/**
 * create ptr type
 */
PtrType::PtrType(Type *base, int level)
: Type(base, TypeKind::Pointer, true), m_level(level)
{}


/**
 * compare ptr type to another type
 */
bool PtrType::equal(Type *type) const
{
    // different kinds
    if (kind() != type->kind()) return false;
    
    // cast
    auto other = static_cast<PtrType *>(type);
    return m_level == other->m_level && getBaseType()->compare(other->getBaseType());
}




/**
 * create function type
 */
FunctionType::FunctionType(Type *result, bool vararg)
: Type(result, TypeKind::Function, false), vararg(vararg)
{}


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
    // different kinds?
    if (kind() != type->kind()) return false;
    
    // cast
    auto other = static_cast<FunctionType *>(type);
    
    // differnt number of arguments?
    if (params.size() != other->params.size()) return false;
    
    // check return type
    if (!getBaseType()->compare(other->getBaseType())) return false;
    
    // Compare the parameters
    return std::equal(params.begin(), params.end(), other->params.begin(), compare_types_pred);
    return false;
}


/**
 * clean up
 */
FunctionType::~FunctionType() {}
