//
// Created by Albert Varaksin on 07/07/2020.
//
#pragma once
#include "pch.h"
#include "Type.def.h"

namespace lbc {

enum class TypeFamily {
    Void,    // Void, lack of typeExpr
    Any,     // any ptr, null
    Pointer, // Ptr to another typeExpr

    Boolean,       // true / false
    Integral,      // signed / unsigned integer 8, 16, 32, ... bits
    FloatingPoint, // single, double

    Function, // function
    ZString,  // nil terminated string, byte ptr / char*
};
#define CHECK_TYPE_FAMILY_NUMERIC_RANGE(val) \
    ((val) >= TypeFamily::Boolean) && ((val) <= TypeFamily::FloatingPoint)


class TypePointer;
class TypeNumeric;
class TypeBoolean;
class TypeIntegral;
class TypeFloatingPoint;
class TypeFunction;
class TypeZString;
class Context;
enum class TokenKind;

/**
 * Base typeExpr is root for all lb types
 *
 * It uses llvm custom rtti system
 */
class TypeRoot : private NonCopyable {
public:
    [[nodiscard]] TypeFamily kind() const noexcept { return m_kind; }

    [[nodiscard]] llvm::Type* llvmType(Context& context) const noexcept {
        if (m_llvmType == nullptr) {
            m_llvmType = genLlvmType(context);
        }
        return m_llvmType;
    }
    virtual ~TypeRoot() = default;
    [[nodiscard]] static const TypeRoot* fromTokenKind(TokenKind kind) noexcept;

    [[nodiscard]] virtual string asString() const noexcept = 0;

protected:
    explicit TypeRoot(TypeFamily kind) noexcept : m_kind{ kind } {}

    [[nodiscard]] virtual llvm::Type* genLlvmType(Context& context) const noexcept = 0;

private:
    mutable llvm::Type* m_llvmType = nullptr;
    const TypeFamily m_kind;
};

/**
 * Void, lack of typeExpr. Cannot be used for C style `void*`
 * Use `Any Ptr` for this
 */
class TypeVoid final : public TypeRoot {
public:
    TypeVoid() noexcept : TypeRoot{ TypeFamily::Void } {}
    static const TypeVoid* get() noexcept;

    static bool classof(const TypeRoot* type) noexcept {
        return type->kind() == TypeFamily::Void;
    }

    [[nodiscard]] string asString() const noexcept final;

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;
};

/**
 * Use `Any Ptr` for this
 */
class TypeAny final : public TypeRoot {
public:
    TypeAny() noexcept : TypeRoot{ TypeFamily::Any } {}
    [[nodiscard]] static const TypeAny* get() noexcept;

    static bool classof(const TypeRoot* type) noexcept {
        return type->kind() == TypeFamily::Any;
    }

    [[nodiscard]] string asString() const noexcept final;

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;
};

/**
 * Pointer to another typeExpr
 */
class TypePointer final : public TypeRoot {
public:
    explicit TypePointer(const TypeRoot* base) noexcept
    : TypeRoot{ TypeFamily::Pointer }, m_base{ base } {}

    [[nodiscard]] static const TypePointer* get(const TypeRoot* base) noexcept;

    static bool classof(const TypeRoot* type) noexcept {
        return type->kind() == TypeFamily::Pointer;
    }

    [[nodiscard]] string asString() const noexcept final;

    [[nodiscard]] const TypeRoot* base() const noexcept { return m_base; }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;

private:
    const TypeRoot* m_base;
};

/**
 * Base for all numeric declaredTypes
 * Bool while conforming, is special kind
 */
class TypeNumeric : public TypeRoot {
protected:
    TypeNumeric(TypeFamily kind, unsigned bits, bool isSigned) noexcept
    : TypeRoot{ kind }, m_bits{ bits }, m_isSigned{ isSigned } {}

public:
    static bool classof(const TypeRoot* type) noexcept {
        return CHECK_TYPE_FAMILY_NUMERIC_RANGE(type->kind());
    }

    [[nodiscard]] unsigned bits() const noexcept { return m_bits; }
    [[nodiscard]] bool isSigned() const noexcept { return m_isSigned; }

private:
    const unsigned m_bits;
    const bool m_isSigned;
};

/**
 * Boolean true / false, result of comparison operators
 */
class TypeBoolean final : public TypeNumeric {
public:
    TypeBoolean() noexcept : TypeNumeric{ TypeFamily::Boolean, 1, false } {}

    [[nodiscard]] static const TypeBoolean* get() noexcept;

    static bool classof(const TypeRoot* type) noexcept {
        return type->kind() == TypeFamily::Boolean;
    }

    [[nodiscard]] string asString() const noexcept final;

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;
};

/**
 * Fixed width integer declaredTypes.
 */
class TypeIntegral final : public TypeNumeric {
public:
    TypeIntegral(unsigned bits, bool isSigned) noexcept
    : TypeNumeric{ TypeFamily::Integral, bits, isSigned } {}

    [[nodiscard]] static const TypeIntegral* get(unsigned bits, bool isSigned) noexcept;

    static bool classof(const TypeRoot* type) noexcept {
        return type->kind() == TypeFamily::Integral;
    }

    [[nodiscard]] string asString() const noexcept final;

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;
};

/**
 * Floating point declaredTypes
 */
class TypeFloatingPoint final : public TypeNumeric {
public:
    explicit TypeFloatingPoint(unsigned bits) noexcept
    : TypeNumeric{ TypeFamily::FloatingPoint, bits, false } {}

    [[nodiscard]] static const TypeFloatingPoint* get(unsigned bits) noexcept;

    static bool classof(const TypeRoot* type) noexcept {
        return type->kind() == TypeFamily::FloatingPoint;
    }

    [[nodiscard]] string asString() const noexcept final;

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;
};

/**
 * Function typeExpr
 */
class TypeFunction final : public TypeRoot {
public:
    TypeFunction(const TypeRoot* retType, std::vector<const TypeRoot*>&& paramTypes, bool variadic) noexcept
    : TypeRoot{ TypeFamily::Function },
      m_retType{ retType },
      m_paramTypes{ std::move(paramTypes) },
      m_variadic{ variadic } {}

    [[nodiscard]] static const TypeFunction* get(
        const TypeRoot* retType,
        std::vector<const TypeRoot*>&& paramTypes,
        bool variadic) noexcept;

    static bool classof(const TypeRoot* type) noexcept {
        return type->kind() == TypeFamily::Function;
    }

    [[nodiscard]] string asString() const noexcept final;

    [[nodiscard]] const TypeRoot* retType() const noexcept { return m_retType; }
    [[nodiscard]] const std::vector<const TypeRoot*>& paramTypes() const noexcept { return m_paramTypes; }
    [[nodiscard]] bool variadic() const noexcept { return m_variadic; }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;

private:
    const TypeRoot* m_retType;
    const std::vector<const TypeRoot*> m_paramTypes;
    const bool m_variadic;
};

/**
 * ZString zero terminated string literal.
 * Equivalent to C `char*`
 */
class TypeZString final : public TypeRoot {
public:
    TypeZString() noexcept : TypeRoot{ TypeFamily::ZString } {}

    [[nodiscard]] static const TypeZString* get() noexcept;

    static bool classof(const TypeRoot* type) noexcept {
        return type->kind() == TypeFamily::ZString;
    }

    [[nodiscard]] string asString() const noexcept final;

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;
};

} // namespace lbc
