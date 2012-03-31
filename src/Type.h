//
//  Type.h
//  LightBASIC
//
//  Created by Albert Varaksin on 29/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

#include "Type.def.h"

namespace llvm {
    class Type;
}

namespace lbc {
    
    // forward declare enum types
    enum class TokenType : int;
    class Symbol;
    
    /**
     * Information about type
     */
    struct Type // : NonCopyable
    {
        /**
         * Type kind.
         */
        enum TypeKind {
            Primitive       = 1 << 0,
            Integral        = 1 << 1,
            FloatingPoint   = 1 << 2,
            Pointer         = 1 << 3,
            Function        = 1 << 4,
            Unsigned        = 1 << 5,
            Instantiable    = 1 << 6,
            Boolean         = 1 << 7,
            AnyPtr          = 1 << 8
        };
        bool isPrimitive()          const { return (m_kind & TypeKind::Primitive) != 0; }
        bool isIntegral()           const { return (m_kind & TypeKind::Integral) != 0; }
        bool isFloatingPoint()      const { return (m_kind & TypeKind::FloatingPoint) != 0; }
        bool isPointer()            const { return (m_kind & TypeKind::Pointer) != 0; }
        bool isFunction()           const { return (m_kind & TypeKind::Function) != 0; }
        bool isUnsignedIntegral()   const { return isIntegral() && (m_kind & TypeKind::Unsigned) != 0; }
        bool isSignedIntegral()     const { return isIntegral() && (m_kind & TypeKind::Unsigned) == 0; }
        bool isInstantiable()       const { return (m_kind & TypeKind::Instantiable) != 0; }
        bool isBoolean()            const { return (m_kind & TypeKind::Boolean) != 0; }
        bool IsAnyPtr()             const { return (m_kind & TypeKind::AnyPtr) != 0; }
        
        // create with base type
        Type(Type *  base, TypeKind kind, bool instantiable);
        
        // clean up
        virtual ~Type();
    
        
        // compare types
        bool compare(Type * ) const;
        
        // return base type
        Type *  getBaseType() const { return m_baseType; }
        
        // get type kind
        TypeKind kind() const { return m_kind; }
        
        // get size
        virtual int getSizeInBits() const { return 0; }
        
        // get string representation of the type
        virtual string toString() = 0;
        
        // get llvm type representing this type
        llvm::Type * llvm() {
            if (!m_llvm) m_llvm = genLlvmType();
            return m_llvm;
        }
        
    protected:
        
        // generate llvm type
        virtual llvm::Type * genLlvmType() = 0;
        
        // are types equal?
        virtual bool equal(Type * ) const = 0;
        
        // base type
        Type *  m_baseType;
        
    private:
        
        // the kind of the token
        TypeKind m_kind;
        
        // associated llvm type
        llvm::Type * m_llvm;
    };
    
    
    /**
     * Primitive types
     */
    struct PrimitiveType : Type
    {
        // get pointer to a basic type
        static Type * get(TokenType type);
        
        // create
        PrimitiveType(TypeKind kind, int size);
        
        // cleanup
        virtual ~PrimitiveType();
        
        // is this equal to another type?
        virtual bool equal(Type *) const;
        
        // get the size
        virtual int getSizeInBits() const { return m_size; }
        
        // get string representation
        virtual string toString();
        
        // get llvm::Type representation
        virtual llvm::Type * genLlvmType();
        
    private:
        int m_size;
    };
    
    
    /**
     * pointer to a type
     */
    struct PtrType : Type
    {
        // get shared pointer type instance
        static PtrType * get(Type *  base, int indirection);
        
        // get any ptr
        static PtrType * getAnyPtr();
        
        // clean up
        virtual ~PtrType();
        
        // is this equal to another type?
        virtual bool equal(Type * ) const;
        
        // get pointer size
        virtual int getSizeInBits() const { return 64; }
        
        // get indirection level
        int indirection() const { return m_level; }
        
        // dereference the pointer by n level. Default to 1
        Type * dereference(int levels = 1) const;
        
        // get string representation
        virtual string toString();
        
        // get llvm::Type representation
        virtual llvm::Type * genLlvmType();
        
    private:
        // create
        PtrType(Type * base, int level, int flags = TypeKind::Pointer);
        
        int m_level;
    };
    
    
    /**
     * Function type
     */
    struct FunctionType : Type
    {
        // create
        FunctionType(Type * result = nullptr, bool vararg = false);
        
        // clean up
        virtual ~FunctionType();
        
        // is this equal to another type?
        virtual bool equal(Type * ) const;
        
        // get function result type
        Type * result() const { return m_baseType; }
        
        // set the function result type
        void result(Type * type) { m_baseType = type; }
        
        // get pointer size
        virtual int getSizeInBits() const { return 64; }
        
        // get string representation
        virtual string toString();
        
        // get llvm::Type representation
        virtual llvm::Type * genLlvmType();
        
        // vararg?
        bool vararg;
        
        // parameters
        vector<Type * > params;
    };
}
