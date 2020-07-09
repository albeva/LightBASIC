//
// Created by Albert on 07/07/2020.
//
#pragma once
#include "pch.h"
#include "Type.def.h"

namespace lbc {

enum class TypeKind {
    Void,               // Void, lack of type
    Any,                // any ptr, null
    Pointer,            // Ptr to another type

    Number,             // A number
    Bool,               // true / false
    Integer,            // signed / unsigned integer 8, 16, 32, ... bits
    FloatingPoint,      // single, double
    NumberLast,         // end of numeric declaredTypes

    Function,           // function
    ZString,            // nil terminated string, byte ptr / char*
};

class Type;
class TypePointer;
class TypeNumber;
class TypeBool;
class TypeInteger;
class TypeFloatingPoint;
class TypeFunction;
class TypeZString;

/**
 * Base type is root for all lb types
 *
 * It uses llvm custom rtti system
 */
class TypeRoot {
    NON_COPYABLE(TypeRoot)
public:
    virtual ~TypeRoot() = default;

    [[nodiscard]] TypeKind kind() const { return m_kind; }

    [[nodiscard]] llvm::Type* llvmType() {
        if (m_llvmType == nullptr) { m_llvmType = genLlvmType(); }
        return m_llvmType;
    }

protected:
    explicit TypeRoot(TypeKind kind) : m_kind{kind} {}

    [[nodiscard]] virtual llvm::Type* genLlvmType() const = 0;

private:
    llvm::Type* m_llvmType = nullptr;
    const TypeKind m_kind;
};

/**
 * Void, lack of type. Cannot be used for C style `void*`
 * Use `Any Ptr` for this
 */
class TypeVoid final: public TypeRoot {
public:
    TypeVoid() : TypeRoot{TypeKind::Void} {}
    static const TypeVoid* get();

    static bool classof(const TypeRoot *type) {
        return type->kind() == TypeKind::Void;
    }

    [[nodiscard]] llvm::Type* genLlvmType() const final;
};

/**
 * Void, lack of type. Cannot be used for C style `void*`
 * Use `Any Ptr` for this
 */
class TypeAny final: public TypeRoot {
public:
    TypeAny() : TypeRoot{TypeKind::Any} {}
    static const TypeAny* get();

    static bool classof(const TypeRoot *type) {
        return type->kind() == TypeKind::Any;
    }

    [[nodiscard]] llvm::Type* genLlvmType() const final;
};

/**
 * Pointer to another type
 */
class TypePointer final: public TypeRoot {
public:
    explicit TypePointer(const TypeRoot* base)
    : TypeRoot{TypeKind::Pointer}, m_base{base} {}

    static const TypePointer* get(const TypeRoot* base);

    static bool classof(const TypeRoot *type) {
        return type->kind() == TypeKind::Pointer;
    }

    [[nodiscard]] llvm::Type* genLlvmType() const final;

    [[nodiscard]] const TypeRoot * base() const { return m_base; }

private:
    const TypeRoot* m_base;
};

/**
 * Base for all numeric declaredTypes
 * Bool while conforming, is special kind
 */
class TypeNumber: public TypeRoot {
protected:
    using TypeRoot::TypeRoot;

public:
    static bool classof(const TypeRoot *type) {
        return type->kind() >= TypeKind::Number &&
               type->kind() < TypeKind::NumberLast;
    }

    [[nodiscard]] virtual int bits() const = 0;
    [[nodiscard]] virtual bool isSigned() const = 0;
};

/**
 * Boolean true / false, result of comparison operators
 */
class TypeBool final: public TypeNumber {
public:
    TypeBool() : TypeNumber{TypeKind::Bool} {}

    static const TypeBool* get();

    static bool classof(const TypeRoot *type) {
        return type->kind() == TypeKind::Bool;
    }

    [[nodiscard]] llvm::Type* genLlvmType() const final;

    [[nodiscard]] int bits() const final { return 1; }
    [[nodiscard]] bool isSigned() const final { return false; }
};

/**
 * Fixed width integer declaredTypes.
 */
class TypeInteger final: public TypeNumber {
public:
    TypeInteger(int bits, bool isSigned)
    : TypeNumber{TypeKind::Integer},
      m_bits{ bits },
      m_isSigned{ isSigned } {}

    static const TypeInteger* get(int bits, bool isSigned);

    static bool classof(const TypeRoot *type) {
        return type->kind() == TypeKind::Integer;
    }

    [[nodiscard]] llvm::Type* genLlvmType() const final;

    [[nodiscard]] int bits() const final { return m_bits; }
    [[nodiscard]] bool isSigned() const final { return m_isSigned; }

private:
    const int m_bits;
    const bool m_isSigned;
};

/**
 * Floating point declaredTypes
 */
class TypeFloatingPoint final: public TypeNumber {
public:
    explicit TypeFloatingPoint(int bits)
    : TypeNumber{TypeKind::FloatingPoint},
      m_bits{bits} {}

    static const TypeFloatingPoint* get(int bits);

    static bool classof(const TypeRoot *type) {
        return type->kind() == TypeKind::FloatingPoint;
    }

    [[nodiscard]] llvm::Type* genLlvmType() const final;

    [[nodiscard]] int bits() const final { return m_bits; }
    [[nodiscard]] bool isSigned() const final { return false; }

private:
    const int m_bits;
};

/**
 * Function type
 */
class TypeFunction final: public TypeRoot {
public:
    TypeFunction(const TypeRoot* retType, std::vector<const TypeRoot*>&& paramTypes)
    : TypeRoot{TypeKind::Function},
      m_retType{retType},
      m_paramTypes{std::move(paramTypes)}
    {}

    static const TypeFunction* get(const TypeRoot* retType, std::vector<const TypeRoot*>&& paramTypes);

    static bool classof(const TypeRoot *type) {
        return type->kind() == TypeKind::Function;
    }

    [[nodiscard]] llvm::Type* genLlvmType() const final;

    [[nodiscard]] const TypeRoot* retType() const { return m_retType; }
    [[nodiscard]] const std::vector<const TypeRoot*>& paramTypes() const { return m_paramTypes; }

private:
    const TypeRoot* m_retType;
    const std::vector<const TypeRoot*> m_paramTypes;
};

/**
 * ZString zero terminated string literal.
 * Equivelant to C `char*`
 */
class TypeZString final: public TypeRoot {
public:
    TypeZString(): TypeRoot{TypeKind::ZString} {}

    static const TypeZString* get();

    static bool classof(const TypeRoot *type) {
        return type->kind() == TypeKind::ZString;
    }

    [[nodiscard]] llvm::Type* genLlvmType() const final;
};


} // namespace lbc
