//
// Created by Albert Varaksin on 05/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast.def.h"
#include "Lexer/Token.h"
#include "Symbol/SymbolTable.h"

namespace lbc {

class TypeRoot;
AST_FORWARD_DECLARE()

enum class AstKind {
#define KIND_ENUM(id, ...) id,
    AST_CONTENT_NODES(KIND_ENUM)
#undef KIND_ENUM
};

/**
 * Root class for all AST nodes. This is an abstract class
 * and should never be used as type for ast node directly
 */
class AstRoot {
public:
    NO_COPY_AND_MOVE(AstRoot)

    explicit AstRoot(AstKind kind) : m_kind{ kind } {}
    virtual ~AstRoot() = default;

    [[nodiscard]] AstKind kind() const { return m_kind; }

    [[nodiscard]] const llvm::StringLiteral& describe() const noexcept;

private:
    const AstKind m_kind;
};

#define IS_AST_CLASSOF(FIRST, LAST) ast->kind() >= AstKind::FIRST && ast->kind() <= AstKind::LAST;

template<typename This, typename Base, AstKind kind>
class AstNode: public Base {
public:
    AstNode() noexcept: Base{ kind } {}

    static bool classof(const AstRoot* ast) noexcept {
        return ast->kind() == kind;
    }

    static unique_ptr<This> create() noexcept {
        return make_unique<This>();
    }
};

//----------------------------------------
// Module
//----------------------------------------

class AstModule final: public AstNode<AstModule, AstRoot, AstKind::Module> {
public:
    unsigned int fileId = ~0U;
    bool hasImplicitMain = false;
    unique_ptr<SymbolTable> symbolTable;
    unique_ptr<AstStmtList> stmtList;
};

//----------------------------------------
// Statements
//----------------------------------------

class AstStmt : public AstRoot {
public:
    using AstRoot::AstRoot;

    static bool classof(const AstRoot* ast) {
        return AST_STMT_RANGE(IS_AST_CLASSOF)
    }
};

class AstStmtList final: public AstNode<AstStmtList, AstStmt, AstKind::StmtList> {
public:
    std::vector<unique_ptr<AstStmt>> stmts;
};

class AstExprStmt final: public AstNode<AstExprStmt, AstStmt, AstKind::ExprStmt> {
public:
    unique_ptr<AstExpr> expr;
};

class AstAssignStmt final: public AstNode<AstAssignStmt, AstStmt, AstKind::AssignStmt> {
public:
    unique_ptr<AstIdentExpr> identExpr;
    unique_ptr<AstExpr> expr;
};

class AstFuncStmt final: public AstNode<AstFuncStmt, AstStmt, AstKind::FuncStmt> {
public:
    unique_ptr<AstFuncDecl> decl;
    unique_ptr<AstStmtList> stmtList;
};

class AstReturnStmt final: public AstNode<AstReturnStmt, AstStmt, AstKind::ReturnStmt> {
public:
    unique_ptr<AstExpr> expr;
};

//----------------------------------------
// Attributes
//----------------------------------------
class AstAttr : public AstRoot {
public:
    using AstRoot::AstRoot;

    static bool classof(const AstRoot* ast) {
        return AST_ATTR_RANGE(IS_AST_CLASSOF)
    }
};

class AstAttributeList final: public AstNode<AstAttributeList, AstAttr, AstKind::AttributeList> {
public:
    [[nodiscard]] std::optional<StringRef> getStringLiteral(const StringRef& key) const;
    std::vector<unique_ptr<AstAttribute>> attribs;
};

class AstAttribute final: public AstNode<AstAttribute, AstAttr, AstKind::Attribute> {
public:
    unique_ptr<AstIdentExpr> identExpr;
    std::vector<unique_ptr<AstLiteralExpr>> argExprs;
};

//----------------------------------------
// Declarations
//----------------------------------------
class AstDecl : public AstStmt {
public:
    using AstStmt::AstStmt;

    static bool classof(const AstRoot* ast){
        return AST_DECL_RANGE(IS_AST_CLASSOF)
    }

    unique_ptr<AstAttributeList> attributes;
    Symbol* symbol = nullptr;
    StringRef id;
};

class AstVarDecl final: public AstNode<AstVarDecl, AstDecl, AstKind::VarDecl> {
public:
    unique_ptr<AstTypeExpr> typeExpr;
    unique_ptr<AstExpr> expr;
};

class AstFuncDecl final: public AstNode<AstFuncDecl, AstDecl, AstKind::FuncDecl> {
public:
    // declared parameters
    std::vector<unique_ptr<AstFuncParamDecl>> paramDecls;
    // is function variadic?
    bool variadic = false;
    // declared typeExpr
    unique_ptr<AstTypeExpr> retTypeExpr;
    // scope symbol table for parameters
    unique_ptr<SymbolTable> symbolTable;
};

class AstFuncParamDecl final: public AstNode<AstFuncParamDecl, AstDecl, AstKind::FuncParamDecl> {
public:
    unique_ptr<AstTypeExpr> typeExpr;
};

//----------------------------------------
// Types
//----------------------------------------
class AstType : public AstRoot {
public:
    using AstRoot::AstRoot;

    static bool classof(const AstRoot* ast) {
        return AST_TYPE_RANGE(IS_AST_CLASSOF)
    }

    const TypeRoot* type = nullptr;
};

class AstTypeExpr final: public AstNode<AstTypeExpr, AstType, AstKind::TypeExpr> {
public:
    TokenKind tokenKind{};
};

//----------------------------------------
// Expressions
//----------------------------------------
class AstExpr : public AstRoot {
public:
    using AstRoot::AstRoot;

    static bool classof(const AstRoot* ast) {
        return AST_EXPR_RANGE(IS_AST_CLASSOF)
    }

    const TypeRoot* type = nullptr;
    llvm::Value* llvmValue = nullptr;
};

class AstIdentExpr final: public AstNode<AstIdentExpr, AstExpr, AstKind::IdentExpr> {
public:
    StringRef id;
    Symbol* symbol = nullptr;
};

class AstCallExpr final: public AstNode<AstCallExpr, AstExpr, AstKind::CallExpr> {
public:
    unique_ptr<AstIdentExpr> identExpr;
    std::vector<unique_ptr<AstExpr>> argExprs;
};

class AstLiteralExpr final: public AstNode<AstLiteralExpr, AstExpr, AstKind::LiteralExpr> {
public:
    using Value = std::variant<StringRef, uint64_t, double, bool, nullptr_t>;
    Value value{};
};

class AstUnaryExpr final: public AstNode<AstUnaryExpr, AstExpr, AstKind::UnaryExpr> {
public:
    TokenKind tokenKind{ 0 };
    unique_ptr<AstExpr> expr;
};

class AstCastExpr final: public AstNode<AstCastExpr, AstExpr, AstKind::CastExpr> {
public:
    unique_ptr<AstExpr> expr;
    unique_ptr<AstTypeExpr> typeExpr;
    bool implicit = false;
};

#undef IS_AST_CLASSOF

} // namespace lbc
