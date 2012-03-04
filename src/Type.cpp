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

static map<TokenType,    shared_ptr<BasicType>> _basicTypes;
static map<Type *,        map<int, shared_ptr<PtrType>>> _ptrTypes;
static struct Creator {
    Creator() {
        // BYTE
        _basicTypes[TokenType::Byte]    = make_shared<BasicType>(shared_ptr<BasicType>(), TokenType::Byte, 1);
        // INTEGER
        _basicTypes[TokenType::Integer]    = make_shared<BasicType>(_basicTypes[TokenType::Byte], TokenType::Integer, 4);
        // PTR
#define PTR_SIZE 8
        _basicTypes[TokenType::Ptr]        = make_shared<BasicType>(shared_ptr<BasicType>(), TokenType::Ptr, PTR_SIZE);
    }
} _creator;


/**
 * Create type
 */
Type::Type(const shared_ptr<Type> & base, TypeKind kind, bool instantiable)
:  m_baseType(base), m_kind(kind), m_instantiable(instantiable), llvmType(nullptr)
{
    
}


/**
 * cleanup
 */
Type::~Type() {}


/**
 * Compare this to another type
 */
bool Type::compare(const shared_ptr<Type> & type) const
{
    if (this == type.get()) return true;
    return this->equal(type);
}



/**
 * create basic type
 */
BasicType::BasicType(const shared_ptr<Type> & base, TokenType kind, int size)
: Type(base, TypeKind::Basic, true), m_kind(kind), m_size(size)
{}


/**
 * clean up
 */
BasicType::~BasicType() {}


/**
 * get basic type
 */
shared_ptr<Type> BasicType::get(TokenType type)
{    
    auto iter = _basicTypes.find(type);
    if (iter != _basicTypes.end()) return iter->second;
    return shared_ptr<Type>();
}


/**
 * is this type equal to the give type?
 */
bool BasicType::equal(const shared_ptr<Type> & type) const
{
    return false;
    return kind() == type->kind() && m_kind == static_pointer_cast<BasicType>(type)->m_kind;
}



/**
 * get shared ptr type instance
 */
shared_ptr<PtrType> PtrType::get(const shared_ptr<Type> & base, int indirection)
{
    auto & inner = _ptrTypes[base.get()];
    auto iter = inner.find(indirection);
    if (iter != inner.end()) return iter->second;

    auto ptr = make_shared<PtrType>(base, indirection);
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
PtrType::PtrType(const shared_ptr<Type> & base, int level)
: Type(base, TypeKind::Ptr, true), m_level(level)
{}


/**
 * compare ptr type to another type
 */
bool PtrType::equal(const shared_ptr<Type> & type) const
{
    // different kinds
    if (kind() != type->kind()) return false;
    
    // cast
    auto other = static_pointer_cast<PtrType>(type);
    return m_level == other->m_level && getBaseType()->compare(other->getBaseType());
}




/**
 * create function type
 */
FunctionType::FunctionType(const shared_ptr<Type> & result)
: Type(result, TypeKind::Function, false)
{}


/**
 * Compare two types
 */
static bool compare_types_pred(const shared_ptr<Type> & typ1, const shared_ptr<Type> & typ2)
{
    return typ1->compare(typ2);
}



/**
 * compare function type to another type
 */
bool FunctionType::equal(const shared_ptr<Type> & type) const
{
    // different kinds?
    if (kind() != type->kind()) return false;
    
    // cast
    auto other = static_pointer_cast<FunctionType>(type);
    
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
