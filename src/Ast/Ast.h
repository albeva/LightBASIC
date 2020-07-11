//
// Created by Albert on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast.def.h"

namespace lbc {

class AstVisitor;
class Token;
class Symbol;
class SymbolTable;
class TypeRoot;
AST_FORWARD_DECLARE()

// Enumerate all possible ast nodes
// This works with LLVM rtti system
enum class AstKind {
#define KIND_ENUM(id, ...) id,
    Stmt,
    AST_STMT_NODES(KIND_ENUM)
        Decl,
    AST_DECL_NODES(KIND_ENUM)
        DeclLast,
    StmtLast,

    AST_ATTRIB_NODES(KIND_ENUM)
        AST_TYPE_NODES(KIND_ENUM)

            Expr,
    AST_EXPR_NODES(KIND_ENUM)
        ExprLast
#undef KIND_ENUM
};

// Base class for all ast nodes
class AstRoot {
    NON_COPYABLE(AstRoot)
public:
    explicit AstRoot(AstKind kind) : m_kind{ kind } {}
    virtual ~AstRoot();
    [[nodiscard]] AstKind kind() const { return m_kind; }
    virtual void accept(AstVisitor* visitor) = 0;

private:
    const AstKind m_kind;
};

// Base class for all statements
class AstStmt : public AstRoot {
    NON_COPYABLE(AstStmt)
public:
    using AstRoot::AstRoot;
    ~AstStmt() override;

    static bool classof(const AstRoot* ast) {
        return ast->kind() >= AstKind::Stmt && ast->kind() < AstKind::StmtLast;
    }
};

// Base class for all expressions
class AstExpr : public AstRoot {
    NON_COPYABLE(AstExpr)
public:
    using AstRoot::AstRoot;
    ~AstExpr() override;

    static bool classof(const AstRoot* ast) {
        return ast->kind() >= AstKind::Expr && ast->kind() < AstKind::ExprLast;
    }

    const TypeRoot* type = nullptr;
};

class AstDecl : public AstStmt {
    NON_COPYABLE(AstDecl)
public:
    using AstStmt::AstStmt;
    ~AstDecl() override;

    static bool classof(const AstRoot* ast) {
        return ast->kind() >= AstKind::Decl && ast->kind() < AstKind::DeclLast;
    }

    unique_ptr<AstAttributeList> attribs;
    Symbol* symbol = nullptr;
};

#define DECLARE_AST(KIND, BASE)                   \
    class Ast##KIND final : public Ast##BASE {    \
        NON_COPYABLE(Ast##KIND)                   \
    public:                                       \
        using Base = Ast##BASE;                   \
        Ast##KIND();                              \
        ~Ast##KIND();                             \
        void accept(AstVisitor* visitor) final;   \
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
DECLARE_AST(AttributeList, Root)
    [[nodiscard]] const Token* getStringLiteral(const string_view& key) const;
    std::vector<unique_ptr<AstAttribute>> attribs;
DECLARE_END

DECLARE_AST(Attribute, Root)
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
DECLARE_AST(TypeExpr, Root)
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
    llvm::Constant* value = nullptr;
DECLARE_END

#undef DECLARE_AST
#undef DECLARE_END

} // namespace lbc
