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
    Boolean, // true / false

    Integral,      // signed / unsigned integer 8, 16, 32, ... bits
    FloatingPoint, // single, double

    Function, // function
    ZString,  // nil terminated string, byte ptr / char*
};

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
class TypeRoot {
public:
    NO_COPY_AND_MOVE(TypeRoot)

    [[nodiscard]] constexpr TypeFamily getKind() const noexcept { return m_kind; }

    [[nodiscard]] llvm::Type* getLlvmType(Context& context) const noexcept {
        if (m_llvmType == nullptr) {
            m_llvmType = genLlvmType(context);
        }
        return m_llvmType;
    }
    virtual ~TypeRoot() = default;
    [[nodiscard]] static const TypeRoot* fromTokenKind(TokenKind kind) noexcept;
    [[nodiscard]] virtual string asString() const noexcept = 0;

    // Type queries
    constexpr bool isVoid() const noexcept { return m_kind == TypeFamily::Void; }
    constexpr bool isAny() const noexcept { return m_kind == TypeFamily::Any; }
    constexpr bool isPointer() const noexcept { return m_kind == TypeFamily::Pointer; }
    constexpr bool isBoolean() const noexcept { return m_kind == TypeFamily::Boolean; }
    constexpr bool isNumeric() const noexcept { return isIntegral() || isFloatingPoint(); }
    constexpr bool isIntegral() const noexcept { return m_kind == TypeFamily::Integral; }
    constexpr bool isFloatingPoint() const noexcept { return m_kind == TypeFamily::FloatingPoint; }
    constexpr bool isFunction() const noexcept { return m_kind == TypeFamily::Function; }
    constexpr bool isZString() const noexcept { return m_kind == TypeFamily::ZString; }
    bool isAnyPointer() const noexcept;
    bool isSignedIntegral() const noexcept;

    // clang-format off
    #define CHECK_TYPE_METHOD(ID, ...) bool is##ID() const noexcept;
    INTEGRAL_TYPES(CHECK_TYPE_METHOD)
    FLOATINGPOINT_TYPES(CHECK_TYPE_METHOD)
    #undef CHECK_TYPE_METHOD
    // clang-format on

protected:
    constexpr explicit TypeRoot(TypeFamily kind) noexcept : m_kind{ kind } {}

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
    constexpr TypeVoid() noexcept : TypeRoot{ TypeFamily::Void } {}
    static const TypeVoid* get() noexcept;

    constexpr static bool classof(const TypeRoot* type) noexcept {
        return type->getKind() == TypeFamily::Void;
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
    constexpr TypeAny() noexcept : TypeRoot{ TypeFamily::Any } {}
    [[nodiscard]] static const TypeAny* get() noexcept;

    constexpr static bool classof(const TypeRoot* type) noexcept {
        return type->getKind() == TypeFamily::Any;
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
    constexpr explicit TypePointer(const TypeRoot* base) noexcept
    : TypeRoot{ TypeFamily::Pointer }, m_base{ base } {}

    [[nodiscard]] static const TypePointer* get(const TypeRoot* base) noexcept;

    constexpr static bool classof(const TypeRoot* type) noexcept {
        return type->getKind() == TypeFamily::Pointer;
    }

    [[nodiscard]] string asString() const noexcept final;

    [[nodiscard]] constexpr const TypeRoot* getBase() const noexcept { return m_base; }

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;

private:
    const TypeRoot* m_base;
};

/**
 * Boolean true / false, result of comparison operators
 */
class TypeBoolean final : public TypeRoot {
public:
    constexpr TypeBoolean() noexcept : TypeRoot{ TypeFamily::Boolean } {}

    [[nodiscard]] static const TypeBoolean* get() noexcept;

    constexpr static bool classof(const TypeRoot* type) noexcept {
        return type->getKind() == TypeFamily::Boolean;
    }

    [[nodiscard]] constexpr unsigned getBits() const noexcept { return 1; }
    [[nodiscard]] constexpr bool isSigned() const noexcept { return false; }

    [[nodiscard]] string asString() const noexcept final;

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;
};

/**
 * Base for all numeric declaredTypes
 * Bool while conforming, is special getKind
 */
class TypeNumeric : public TypeRoot {
public:
    constexpr static bool classof(const TypeRoot* type) noexcept {
        auto kind = type->getKind();
        return kind == TypeFamily::Integral || kind == TypeFamily::FloatingPoint;
    }

    [[nodiscard]] constexpr unsigned getBits() const noexcept { return m_bits; }

protected:
    constexpr TypeNumeric(TypeFamily kind, unsigned bits) noexcept
    : TypeRoot{ kind }, m_bits{ bits } {}

private:
    const unsigned m_bits;
};

/**
 * Fixed width integer declaredTypes.
 */
class TypeIntegral final : public TypeNumeric {
public:
    constexpr TypeIntegral(unsigned bits, bool isSigned) noexcept
    : TypeNumeric{ TypeFamily::Integral, bits }, m_signed{ isSigned } {}

    [[nodiscard]] static const TypeIntegral* get(unsigned bits, bool isSigned) noexcept;

    constexpr static bool classof(const TypeRoot* type) noexcept {
        return type->getKind() == TypeFamily::Integral;
    }

    [[nodiscard]] constexpr bool isSigned() const noexcept { return m_signed; }

    [[nodiscard]] string asString() const noexcept final;

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;

private:
    const bool m_signed;
};

/**
 * Floating point declaredTypes
 */
class TypeFloatingPoint final : public TypeNumeric {
public:
    constexpr explicit TypeFloatingPoint(unsigned bits) noexcept
    : TypeNumeric{ TypeFamily::FloatingPoint, bits } {}

    [[nodiscard]] static const TypeFloatingPoint* get(unsigned bits) noexcept;

    constexpr static bool classof(const TypeRoot* type) noexcept {
        return type->getKind() == TypeFamily::FloatingPoint;
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

    constexpr static bool classof(const TypeRoot* type) noexcept {
        return type->getKind() == TypeFamily::Function;
    }

    [[nodiscard]] string asString() const noexcept final;

    [[nodiscard]] const TypeRoot* getReturn() const noexcept { return m_retType; }
    [[nodiscard]] const std::vector<const TypeRoot*>& getParams() const noexcept { return m_paramTypes; }
    [[nodiscard]] bool isVariadic() const noexcept { return m_variadic; }

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
    constexpr TypeZString() noexcept : TypeRoot{ TypeFamily::ZString } {}

    [[nodiscard]] static const TypeZString* get() noexcept;

    constexpr static bool classof(const TypeRoot* type) noexcept {
        return type->getKind() == TypeFamily::ZString;
    }

    [[nodiscard]] string asString() const noexcept final;

protected:
    [[nodiscard]] llvm::Type* genLlvmType(Context& context) const noexcept final;
};

} // namespace lbc
