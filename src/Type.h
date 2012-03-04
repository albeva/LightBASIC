//
//  Type.h
//  LightBASIC
//
//  Created by Albert Varaksin on 29/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#pragma once

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
    struct Type : NonCopyable
    {
        
        enum TypeKind {
            Basic,
            Ptr,
            Function
        };
        
        // create with base type
        Type(const shared_ptr<Type> & base, TypeKind kind, bool instantiable);
        
        // clean up
        virtual ~Type();
        
        // can instantiate
        bool isInstantiable() const { return m_instantiable; }
        
        // compare types
        bool compare(const shared_ptr<Type> &) const;
        
        // return base type
        shared_ptr<Type> getBaseType() const { return m_baseType; }
        
        // get type kind
        TypeKind kind() const { return m_kind; }
        
        // get size
        virtual int getSizeInBits() const { return 0; }
        
        // associated llvm type
        llvm::Type * llvmType;
        
    protected:
        
        // are types equal?
        virtual bool equal(const shared_ptr<Type> &) const = 0;
        
        // base type
        shared_ptr<Type> m_baseType;
        
    private:
        // can instantiate this type?
        unsigned m_instantiable: 1;
        
        TypeKind m_kind:4;
    };
    
    
    /**
     * Basic type byte, short, integer, long, float, double, bool
     */
    struct BasicType : Type
    {
        // create
        BasicType(const shared_ptr<Type> & base, TokenType kind, int size);
        
        // cleanup
        virtual ~BasicType();
        
        // is this equal to another type?
        virtual bool equal(const shared_ptr<Type> &) const;
        
        // get pointer to a basic type
        static shared_ptr<Type> get(TokenType type);
        
        // get the size
        virtual int getSizeInBits() const { return m_size * 8; }
        
    private:
        TokenType m_kind;
        int m_size; // in bytes
    };
    
    
    /**
     * pointer to a type
     */
    struct PtrType : Type
    {
        // create
        PtrType(const shared_ptr<Type> & base, int level);
        
        // clean up
        virtual ~PtrType();
        
        // get shared pointer type instance
        static shared_ptr<PtrType> get(const shared_ptr<Type> & base, int indirection);
        
        // is this equal to another type?
        virtual bool equal(const shared_ptr<Type> &) const;
        
        // get pointer size
        virtual int getSizeInBits() const { return 64; }
        
        // get indirection level
        int indirection() const { return m_level; }
        
    private:
        int m_level;
    };
    
    
    /**
     * Function type
     */
    struct FunctionType : Type
    {
        // create
        FunctionType(const shared_ptr<Type> & result);
        
        // clean up
        virtual ~FunctionType();
        
        // is this equal to another type?
        virtual bool equal(const shared_ptr<Type> &) const;
        
        // get function result type
        shared_ptr<Type> result() const { return m_baseType; }
        
        // set the function result type
        void result(const shared_ptr<Type> & type) { m_baseType = type; }
        
        // parameters
        vector<shared_ptr<Type>> params;
    };
}
