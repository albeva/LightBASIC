//
// Created by Albert on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast.def.h"

namespace lbc {

class Token;
class Symbol;
class SymbolTable;
class TypeRoot;
AST_FORWARD_DECLARE()

// clang-format off

// Enumerate all possible ast nodes
// This works with LLVM rtti system
enum class AstKind {
#define KIND_ENUM(id, ...) id,
    StmtFirst,
        AST_STMT_NODES(KIND_ENUM)
        DeclFirst,
            AST_DECL_NODES(KIND_ENUM)
        DeclLast,
    StmtLast,

    AttrFirst,
        AST_ATTRIB_NODES(KIND_ENUM)
    AttrLast,

    TypeFirst,
        AST_TYPE_NODES(KIND_ENUM)
    TypeLast,

    ExprFirst,
        AST_EXPR_NODES(KIND_ENUM)
    ExprLast
#undef KIND_ENUM
};

// clang-format on

/**
 * Root class for all AST nodes. This is an abstract class
 * and should never be used as type for ast node directly
 */
class AstRoot {
    NON_COPYABLE(AstRoot)
public:
    explicit AstRoot(AstKind kind) : m_kind{ kind } {}
    virtual ~AstRoot();
    [[nodiscard]] AstKind kind() const { return m_kind; }
private:
    const AstKind m_kind;
};

/**
 * Base class for all statement nodes
 * Declarations also fall under statement!
 */
class AstStmt : public AstRoot {
    NON_COPYABLE(AstStmt)
public:
    using AstRoot::AstRoot;
    ~AstStmt() override;

    static bool classof(const AstRoot* ast) {
        return ast->kind() > AstKind::StmtFirst && ast->kind() < AstKind::StmtLast;
    }
};

/**
 * Base class for all expression nodes
 */
class AstExpr : public AstRoot {
    NON_COPYABLE(AstExpr)
public:
    using AstRoot::AstRoot;
    ~AstExpr() override;

    static bool classof(const AstRoot* ast) {
        return ast->kind() > AstKind::ExprFirst && ast->kind() < AstKind::ExprLast;
    }

    const TypeRoot* type = nullptr;
    llvm::Value* llvmValue = nullptr;
};

/**
 * Base class for attribute nodes.
 */
class AstAttr : public AstRoot {
    NON_COPYABLE(AstAttr)
public:
    using AstRoot::AstRoot;
    ~AstAttr() override;

    static bool classof(const AstRoot* ast) {
        return ast->kind() > AstKind::AttrFirst && ast->kind() < AstKind::AttrLast;
    }
};

/**
 * Base class for type expressions
 */
class AstType : public AstRoot {
    NON_COPYABLE(AstType)
public:
    using AstRoot::AstRoot;
    ~AstType() override;

    static bool classof(const AstRoot* ast) {
        return ast->kind() > AstKind::TypeFirst && ast->kind() < AstKind::TypeLast;
    }
};

/**
 * Base class for declaration nodes
 */
class AstDecl : public AstStmt {
    NON_COPYABLE(AstDecl)
public:
    using AstStmt::AstStmt;
    ~AstDecl() override;

    static bool classof(const AstRoot* ast) {
        return ast->kind() > AstKind::DeclFirst && ast->kind() < AstKind::DeclLast;
    }

    unique_ptr<AstAttributeList> attributes;
    Symbol* symbol = nullptr;
};

#define DECLARE_AST(KIND, BASE)                   \
    class Ast##KIND final : public Ast##BASE {    \
        NON_COPYABLE(Ast##KIND)                   \
    public:                                       \
        using Base = Ast##BASE;                   \
        Ast##KIND();                              \
        ~Ast##KIND();                             \
        static bool classof(const AstRoot* ast) { \
            return ast->kind() == AstKind::KIND;  \
        }                                         \
        static unique_ptr<Ast##KIND> create() {   \
            return make_unique<Ast##KIND>();      \
        }

#define DECLARE_END \
    }               \
    ;

//----------------------------------------
// Statements
//----------------------------------------

DECLARE_AST(Program, Stmt)
    unique_ptr<SymbolTable> symbolTable;
    unique_ptr<AstStmtList> stmtList;
DECLARE_END

DECLARE_AST(StmtList, Stmt)
    std::vector<unique_ptr<AstStmt>> stmts;
DECLARE_END

DECLARE_AST(ExprStmt, Stmt)
    unique_ptr<AstExpr> expr;
DECLARE_END

DECLARE_AST(AssignStmt, Stmt)
    unique_ptr<AstIdentExpr> identExpr;
    unique_ptr<AstExpr> expr;
DECLARE_END

//----------------------------------------
// Attributes
//----------------------------------------
DECLARE_AST(AttributeList, Attr)
    [[nodiscard]] const Token* getStringLiteral(const string_view& key) const;
    std::vector<unique_ptr<AstAttribute>> attribs;
DECLARE_END

DECLARE_AST(Attribute, Attr)
    unique_ptr<AstIdentExpr> identExpr;
    std::vector<unique_ptr<AstLiteralExpr>> argExprs;
DECLARE_END

//----------------------------------------
// Declarations
//----------------------------------------

DECLARE_AST(VarDecl, Decl)
    unique_ptr<Token> token;
    unique_ptr<AstTypeExpr> typeExpr;
    unique_ptr<AstExpr> expr;
DECLARE_END

DECLARE_AST(FuncDecl, Decl)
    // identifier
    unique_ptr<Token> token;
    // declared parameters
    std::vector<unique_ptr<AstFuncParamDecl>> paramDecls;
    // is function variadic?
    bool variadic = false;
    // declared typeExpr
    unique_ptr<AstTypeExpr> retTypeExpr;
    // scope symbol table for parameters
    unique_ptr<SymbolTable> symbolTable;
DECLARE_END

DECLARE_AST(FuncParamDecl, Decl)
    unique_ptr<Token> token;
    unique_ptr<AstTypeExpr> typeExpr;
DECLARE_END

//----------------------------------------
// Types
//----------------------------------------
DECLARE_AST(TypeExpr, Type)
    unique_ptr<Token> token;
    const TypeRoot* type = nullptr;
DECLARE_END

//----------------------------------------
// Expressions
//----------------------------------------

DECLARE_AST(IdentExpr, Expr)
    unique_ptr<Token> token;
    Symbol* symbol = nullptr;
DECLARE_END

DECLARE_AST(CallExpr, Expr)
    unique_ptr<AstIdentExpr> identExpr;
    std::vector<unique_ptr<AstExpr>> argExprs;
DECLARE_END

DECLARE_AST(LiteralExpr, Expr)
    unique_ptr<Token> token;
DECLARE_END

#undef DECLARE_AST
#undef DECLARE_END

} // namespace lbc
