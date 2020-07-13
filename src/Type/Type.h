//
// Created by Albert on 07/07/2020.
//
#pragma once
#include "pch.h"
#include "Type.def.h"

namespace lbc {

enum class TypeKind {
    Void,    // Void, lack of typeExpr
    Any,     // any ptr, null
    Pointer, // Ptr to another typeExpr

    Number,        // A number
    Bool,          // true / false
    Integer,       // signed / unsigned integer 8, 16, 32, ... bits
    FloatingPoint, // single, double
    NumberLast,    // end of numeric declaredTypes

    Function, // function
    ZString,  // nil terminated string, byte ptr / char*
};

class Type;
class TypePointer;
class TypeNumber;
class TypeBool;
class TypeInteger;
class TypeFloatingPoint;
class TypeFunction;
class TypeZString;
enum class TokenKind;

/**
 * Base typeExpr is root for all lb types
 *
 * It uses llvm custom rtti system
 */
class TypeRoot {
    NON_COPYABLE(TypeRoot)
public:
    virtual ~TypeRoot();

    [[nodiscard]] TypeKind kind() const { return m_kind; }

    [[nodiscard]] llvm::Type* llvmType(llvm::LLVMContext& context) const {
        if (m_llvmType == nullptr) {
            m_llvmType = genLlvmType(context);
        }
        return m_llvmType;
    }

    [[nodiscard]] static const TypeRoot* fromTokenKind(TokenKind kind);

protected:
    explicit TypeRoot(TypeKind kind): m_kind{kind} {}

    [[nodiscard]] virtual llvm::Type* genLlvmType(llvm::LLVMContext& context) const = 0;

private:
    mutable llvm::Type* m_llvmType = nullptr;
    const TypeKind m_kind;
};

/**
 * Void, lack of typeExpr. Cannot be used for C style `void*`
 * Use `Any Ptr` for this
 */
class TypeVoid final : public TypeRoot {
public:
    TypeVoid() : TypeRoot{ TypeKind::Void } {}
    static const TypeVoid* get();

    static bool classof(const TypeRoot* type) {
        return type->kind() == TypeKind::Void;
    }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(llvm::LLVMContext& context) const final;
};

/**
 * Use `Any Ptr` for this
 */
class TypeAny final : public TypeRoot {
public:
    TypeAny() : TypeRoot{ TypeKind::Any } {}
    [[nodiscard]] static const TypeAny* get();

    static bool classof(const TypeRoot* type) {
        return type->kind() == TypeKind::Any;
    }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(llvm::LLVMContext& context) const final;
};

/**
 * Pointer to another typeExpr
 */
class TypePointer final : public TypeRoot {
public:
    explicit TypePointer(const TypeRoot* base)
      : TypeRoot{ TypeKind::Pointer }, m_base{ base } {}

    [[nodiscard]] static const TypePointer* get(const TypeRoot* base);

    static bool classof(const TypeRoot* type) {
        return type->kind() == TypeKind::Pointer;
    }

    [[nodiscard]] const TypeRoot* base() const { return m_base; }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(llvm::LLVMContext& context) const final;

private:
    const TypeRoot* m_base;
};

/**
 * Base for all numeric declaredTypes
 * Bool while conforming, is special kind
 */
class TypeNumber : public TypeRoot {
    NON_COPYABLE(TypeNumber)
protected:
    TypeNumber(TypeKind kind, unsigned bits, bool isSigned)
        : TypeRoot{kind}, m_bits{bits}, m_isSigned{isSigned} {}

public:
    static bool classof(const TypeRoot* type) {
        return type->kind() >= TypeKind::Number && type->kind() < TypeKind::NumberLast;
    }

    ~TypeNumber() override;

    [[nodiscard]] unsigned bits() const { return m_bits; }
    [[nodiscard]] bool isSigned() const { return m_isSigned; }

private:
    const unsigned m_bits;
    const bool m_isSigned;
};

/**
 * Boolean true / false, result of comparison operators
 */
class TypeBool final : public TypeNumber {
public:
    TypeBool() : TypeNumber{ TypeKind::Bool, 1, false } {}

    [[nodiscard]] static const TypeBool* get();

    static bool classof(const TypeRoot* type) {
        return type->kind() == TypeKind::Bool;
    }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(llvm::LLVMContext& context) const final;
};

/**
 * Fixed width integer declaredTypes.
 */
class TypeInteger final : public TypeNumber {
public:
    TypeInteger(unsigned bits, bool isSigned)
      : TypeNumber{ TypeKind::Integer, bits, isSigned } {}

    [[nodiscard]] static const TypeInteger* get(unsigned bits, bool isSigned);

    static bool classof(const TypeRoot* type) {
        return type->kind() == TypeKind::Integer;
    }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(llvm::LLVMContext& context) const final;
};

/**
 * Floating point declaredTypes
 */
class TypeFloatingPoint final : public TypeNumber {
public:
    explicit TypeFloatingPoint(unsigned bits)
      : TypeNumber{ TypeKind::FloatingPoint, bits, false } {}

    [[nodiscard]] static const TypeFloatingPoint* get(unsigned bits);

    static bool classof(const TypeRoot* type) {
        return type->kind() == TypeKind::FloatingPoint;
    }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(llvm::LLVMContext& context) const final;
};

/**
 * Function typeExpr
 */
class TypeFunction final : public TypeRoot {
public:
    TypeFunction(const TypeRoot* retType, std::vector<const TypeRoot*>&& paramTypes)
      : TypeRoot{ TypeKind::Function },
        m_retType{ retType },
        m_paramTypes{ std::move(paramTypes) } {}

    [[nodiscard]] static const TypeFunction* get(const TypeRoot* retType, std::vector<const TypeRoot*>&& paramTypes);

    static bool classof(const TypeRoot* type) {
        return type->kind() == TypeKind::Function;
    }

    [[nodiscard]] const TypeRoot* retType() const { return m_retType; }
    [[nodiscard]] const std::vector<const TypeRoot*>& paramTypes() const { return m_paramTypes; }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(llvm::LLVMContext& context) const final;

private:
    const TypeRoot* m_retType;
    const std::vector<const TypeRoot*> m_paramTypes;
};

/**
 * ZString zero terminated string literal.
 * Equivelant to C `char*`
 */
class TypeZString final : public TypeRoot {
public:
    TypeZString() : TypeRoot{ TypeKind::ZString } {}

    [[nodiscard]] static const TypeZString* get();

    static bool classof(const TypeRoot* type) {
        return type->kind() == TypeKind::ZString;
    }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(llvm::LLVMContext& context) const final;
};


} // namespace lbc
