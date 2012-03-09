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
            Instantiable    = 1 << 6
		};
        bool isPrimitive() const { return (m_kind & TypeKind::Primitive) != 0; }
        bool isIntegral() const { return (m_kind & TypeKind::Integral) != 0; }
        bool isFloatingPoint() const { return (m_kind & TypeKind::FloatingPoint) != 0; }
        bool isPointer() const { return (m_kind & TypeKind::Pointer) != 0; }
        bool isFunction() const { return (m_kind & TypeKind::Function) != 0; }
        bool isUnsignedIntegral() const { return isIntegral() && (m_kind & TypeKind::Unsigned) != 0; }
        bool isSignedIntegral() const { return isIntegral() && (m_kind & TypeKind::Unsigned) == 0; }
        bool isInstantiable() const { return (m_kind & TypeKind::Instantiable) != 0; }
		
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
		
		// associated llvm type
		llvm::Type * llvmType;
		
	protected:
		
		// are types equal?
		virtual bool equal(Type * ) const = 0;
		
		// base type
		Type *  m_baseType;
		
	private:
		// the kind of the token
		TypeKind m_kind;
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
		
	private:
		int m_size;
	};
	
	
	/**
	 * pointer to a type
	 */
	struct PtrType : Type
	{
		// get shared pointer type instance
        static Type * get(Type *  base, int indirection);
        
		// create
		PtrType(Type * base, int level);
		
		// clean up
		virtual ~PtrType();
		
		// is this equal to another type?
		virtual bool equal(Type * ) const;
		
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
		FunctionType(Type * result = nullptr);
		
		// clean up
		virtual ~FunctionType();
		
		// is this equal to another type?
		virtual bool equal(Type * ) const;
		
		// get function result type
		Type *  result() const { return m_baseType; }
		
		// set the function result type
		void result(Type * type) { m_baseType = type; }
        
		// get pointer size
		virtual int getSizeInBits() const { return 64; }
		
		// parameters
		vector<Type * > params;
	};
}
