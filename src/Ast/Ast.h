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
struct AstRoot {
    NO_COPY_AND_MOVE(AstRoot)

    constexpr explicit AstRoot(AstKind kind, llvm::SMRange range) noexcept
    : m_kind{ kind }, m_range{ range } {}
    virtual ~AstRoot() = default;

    [[nodiscard]] constexpr AstKind kind() const noexcept { return m_kind; }
    [[nodiscard]] constexpr llvm::SMRange getRange() const noexcept { return m_range; }
    [[nodiscard]] llvm::StringRef getClassName() const noexcept;

private:
    const AstKind m_kind;
    const llvm::SMRange m_range;
};

template<typename This, typename Base, AstKind kind>
struct AstNode : Base {
    constexpr explicit AstNode(llvm::SMRange range) noexcept : Base{ kind, range } {}

    static constexpr bool classof(const AstRoot* ast) noexcept {
        return ast->kind() == kind;
    }

    constexpr static unique_ptr<This> create(llvm::SMRange range) noexcept {
        return make_unique<This>(range);
    }
};

#define IS_AST_CLASSOF(FIRST, LAST) ast->kind() >= AstKind::FIRST && ast->kind() <= AstKind::LAST;

//----------------------------------------
// Module
//----------------------------------------

struct AstModule final : AstNode<AstModule, AstRoot, AstKind::Module> {
    using AstNode::AstNode;
    unsigned int fileId = ~0U;
    bool hasImplicitMain = false;
    unique_ptr<SymbolTable> symbolTable;
    unique_ptr<AstStmtList> stmtList;
};

//----------------------------------------
// Statements
//----------------------------------------

struct AstStmt : AstRoot {
    using AstRoot::AstRoot;

    static constexpr bool classof(const AstRoot* ast) noexcept {
        return AST_STMT_RANGE(IS_AST_CLASSOF)
    }
};

struct AstStmtList final : AstNode<AstStmtList, AstStmt, AstKind::StmtList> {
    using AstNode::AstNode;
    std::vector<unique_ptr<AstStmt>> stmts;
};

struct AstExprStmt final : AstNode<AstExprStmt, AstStmt, AstKind::ExprStmt> {
    using AstNode::AstNode;
    unique_ptr<AstExpr> expr;
};

struct AstAssignStmt final : AstNode<AstAssignStmt, AstStmt, AstKind::AssignStmt> {
    using AstNode::AstNode;
    unique_ptr<AstIdentExpr> identExpr;
    unique_ptr<AstExpr> expr;
};

struct AstFuncStmt final : AstNode<AstFuncStmt, AstStmt, AstKind::FuncStmt> {
    using AstNode::AstNode;
    unique_ptr<AstFuncDecl> decl;
    unique_ptr<AstStmtList> stmtList;
};

struct AstReturnStmt final : AstNode<AstReturnStmt, AstStmt, AstKind::ReturnStmt> {
    using AstNode::AstNode;
    unique_ptr<AstExpr> expr;
};

//----------------------------------------
// Attributes
//----------------------------------------
struct AstAttr : AstRoot {
    using AstRoot::AstRoot;

    static constexpr bool classof(const AstRoot* ast) noexcept {
        return AST_ATTR_RANGE(IS_AST_CLASSOF)
    }
};

struct AstAttributeList final : AstNode<AstAttributeList, AstAttr, AstKind::AttributeList> {
    using AstNode::AstNode;
    [[nodiscard]] std::optional<StringRef> getStringLiteral(const StringRef& key) const;
    std::vector<unique_ptr<AstAttribute>> attribs;
};

struct AstAttribute final : AstNode<AstAttribute, AstAttr, AstKind::Attribute> {
    using AstNode::AstNode;
    unique_ptr<AstIdentExpr> identExpr;
    std::vector<unique_ptr<AstLiteralExpr>> argExprs;
};

//----------------------------------------
// Declarations
//----------------------------------------
struct AstDecl : AstStmt {
    using AstStmt::AstStmt;

    static constexpr bool classof(const AstRoot* ast) noexcept {
        return AST_DECL_RANGE(IS_AST_CLASSOF)
    }

    unique_ptr<AstAttributeList> attributes;
    Symbol* symbol = nullptr;
    StringRef id;
};

struct AstVarDecl final : AstNode<AstVarDecl, AstDecl, AstKind::VarDecl> {
    using AstNode::AstNode;
    unique_ptr<AstTypeExpr> typeExpr;
    unique_ptr<AstExpr> expr;
};

struct AstFuncDecl final : AstNode<AstFuncDecl, AstDecl, AstKind::FuncDecl> {
    using AstNode::AstNode;
    // declared parameters
    std::vector<unique_ptr<AstFuncParamDecl>> paramDecls;
    // is function variadic?
    bool variadic = false;
    // declared typeExpr
    unique_ptr<AstTypeExpr> retTypeExpr;
    // scope symbol table for parameters
    unique_ptr<SymbolTable> symbolTable;
};

struct AstFuncParamDecl final : AstNode<AstFuncParamDecl, AstDecl, AstKind::FuncParamDecl> {
    using AstNode::AstNode;
    unique_ptr<AstTypeExpr> typeExpr;
};

//----------------------------------------
// Types
//----------------------------------------
struct AstType : AstRoot {
    using AstRoot::AstRoot;

    static constexpr bool classof(const AstRoot* ast) noexcept {
        return AST_TYPE_RANGE(IS_AST_CLASSOF)
    }

    const TypeRoot* type = nullptr;
};

struct AstTypeExpr final : AstNode<AstTypeExpr, AstType, AstKind::TypeExpr> {
    using AstNode::AstNode;
    TokenKind tokenKind{};
};

//----------------------------------------
// Expressions
//----------------------------------------
struct AstExpr : AstRoot {
    using AstRoot::AstRoot;

    static constexpr bool classof(const AstRoot* ast) noexcept {
        return AST_EXPR_RANGE(IS_AST_CLASSOF)
    }

    const TypeRoot* type = nullptr;
    llvm::Value* llvmValue = nullptr;
};

struct AstIdentExpr final : AstNode<AstIdentExpr, AstExpr, AstKind::IdentExpr> {
    using AstNode::AstNode;
    StringRef id;
    Symbol* symbol = nullptr;
};

struct AstCallExpr final : AstNode<AstCallExpr, AstExpr, AstKind::CallExpr> {
    using AstNode::AstNode;
    unique_ptr<AstIdentExpr> identExpr;
    std::vector<unique_ptr<AstExpr>> argExprs;
};

struct AstLiteralExpr final : AstNode<AstLiteralExpr, AstExpr, AstKind::LiteralExpr> {
    using AstNode::AstNode;
    using Value = Token::Value;
    Value value{};
};

struct AstUnaryExpr final : AstNode<AstUnaryExpr, AstExpr, AstKind::UnaryExpr> {
    using AstNode::AstNode;
    TokenKind tokenKind{ 0 };
    unique_ptr<AstExpr> expr;
};

struct AstBinaryExpr final : AstNode<AstBinaryExpr, AstExpr, AstKind::BinaryExpr> {
    using AstNode::AstNode;
    TokenKind tokenKind{ 0 };
    unique_ptr<AstExpr> lhs;
    unique_ptr<AstExpr> rhs;
};

struct AstCastExpr final : AstNode<AstCastExpr, AstExpr, AstKind::CastExpr> {
    using AstNode::AstNode;
    unique_ptr<AstExpr> expr;
    unique_ptr<AstTypeExpr> typeExpr;
    bool implicit = false;
};

struct AstIfExpr final: AstNode<AstIfExpr, AstExpr, AstKind::IfExpr> {
    using AstNode::AstNode;
    unique_ptr<AstExpr> expr;
    unique_ptr<AstExpr> trueExpr;
    unique_ptr<AstExpr> falseExpr;
};

#undef IS_AST_CLASSOF

} // namespace lbc
