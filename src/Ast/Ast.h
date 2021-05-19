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
    virtual ~AstRoot() noexcept = default;

    [[nodiscard]] constexpr AstKind kind() const noexcept { return m_kind; }
    [[nodiscard]] constexpr llvm::SMRange getRange() const noexcept { return m_range; }
    [[nodiscard]] StringRef getClassName() const noexcept;

private:
    const AstKind m_kind;
    const llvm::SMRange m_range;
};

template<typename This, typename Base, AstKind kind>
struct AstNode : Base {
    using Base::Base;
    constexpr static AstKind ID = kind;

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind() == kind;
    }

    template<typename... Args>
    constexpr static unique_ptr<This> create(Args&&... args) noexcept {
        return make_unique<This>(std::forward<Args>(args)...);
    }
};

#define IS_AST_CLASSOF(FIRST, LAST) ast->kind() >= AstKind::FIRST && ast->kind() <= AstKind::LAST;

//----------------------------------------
// Module
//----------------------------------------

struct AstModule final : AstNode<AstModule, AstRoot, AstKind::Module> {
    explicit AstModule(
        unsigned int file,
        llvm::SMRange range,
        bool implicitMain,
        unique_ptr<AstStmtList> stms) noexcept
    : AstNode{ ID, range },
      fileId{ file },
      hasImplicitMain{ implicitMain },
      stmtList{ std::move(stms) } {};

    unsigned int fileId;
    bool hasImplicitMain;
    unique_ptr<AstStmtList> stmtList;
    unique_ptr<SymbolTable> symbolTable;
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
    explicit AstStmtList(
        llvm::SMRange range,
        std::vector<unique_ptr<AstStmt>> list) noexcept
    : AstNode{ ID, range },
      stmts{ std::move(list) } {};

    std::vector<unique_ptr<AstStmt>> stmts;
};

struct AstExprStmt final : AstNode<AstExprStmt, AstStmt, AstKind::ExprStmt> {
    explicit AstExprStmt(
        llvm::SMRange range,
        unique_ptr<AstExpr> e) noexcept
    : AstNode{ ID, range },
      expr{ std::move(e) } {};

    unique_ptr<AstExpr> expr;
};

struct AstAssignStmt final : AstNode<AstAssignStmt, AstStmt, AstKind::AssignStmt> {
    explicit AstAssignStmt(
        llvm::SMRange range,
        unique_ptr<AstIdentExpr> ident,
        unique_ptr<AstExpr> e) noexcept
    : AstNode{ ID, range },
      identExpr{ std::move(ident) },
      expr{ std::move(e) } {};

    unique_ptr<AstIdentExpr> identExpr;
    unique_ptr<AstExpr> expr;
};

struct AstFuncStmt final : AstNode<AstFuncStmt, AstStmt, AstKind::FuncStmt> {
    explicit AstFuncStmt(
        llvm::SMRange range,
        unique_ptr<AstFuncDecl> d,
        unique_ptr<AstStmtList> list) noexcept
    : AstNode{ ID, range },
      decl{ std::move(d) },
      stmtList{ std::move(list) } {};

    unique_ptr<AstFuncDecl> decl;
    unique_ptr<AstStmtList> stmtList;
};

struct AstReturnStmt final : AstNode<AstReturnStmt, AstStmt, AstKind::ReturnStmt> {
    explicit AstReturnStmt(
        llvm::SMRange range,
        unique_ptr<AstExpr> e) noexcept
    : AstNode{ ID, range },
      expr{ std::move(e) } {};

    unique_ptr<AstExpr> expr;
};

struct AstIfStmt final : AstNode<AstIfStmt, AstStmt, AstKind::IfStmt> {
    struct Block final {
        std::vector<unique_ptr<AstVarDecl>> decls;
        unique_ptr<SymbolTable> symbolTable;
        unique_ptr<AstExpr> expr;
        unique_ptr<AstStmt> stmt;
    };

    explicit AstIfStmt(
        llvm::SMRange range,
        std::vector<Block> list) noexcept
    : AstNode{ ID, range },
      blocks{ std::move(list) } {};

    std::vector<Block> blocks;
};

struct AstForStmt final : AstNode<AstForStmt, AstStmt, AstKind::ForStmt> {
    explicit AstForStmt(
        llvm::SMRange range,
        std::vector<unique_ptr<AstVarDecl>> decls_,
        unique_ptr<AstVarDecl> iter,
        unique_ptr<AstExpr> limit_,
        unique_ptr<AstExpr> step_,
        unique_ptr<AstStmt> stmt_,
        StringRef next_) noexcept
    : AstNode{ ID, range },
      decls{ std::move(decls_) },
      iterator{ std::move(iter) },
      limit{ std::move(limit_) },
      step{ std::move(step_) },
      stmt{ std::move(stmt_) },
      next{ next_ } {};

    std::vector<unique_ptr<AstVarDecl>> decls;
    unique_ptr<AstVarDecl> iterator;
    unique_ptr<AstExpr> limit;
    unique_ptr<AstExpr> step;
    unique_ptr<AstStmt> stmt;
    StringRef next;
    unique_ptr<SymbolTable> symbolTable;
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
    explicit AstAttributeList(
        llvm::SMRange range,
        std::vector<unique_ptr<AstAttribute>> attribs_) noexcept
    : AstNode{ ID, range },
      attribs{ std::move(attribs_) } {};

    [[nodiscard]] std::optional<StringRef> getStringLiteral(StringRef key) const;
    std::vector<unique_ptr<AstAttribute>> attribs;
};

struct AstAttribute final : AstNode<AstAttribute, AstAttr, AstKind::Attribute> {
    explicit AstAttribute(
        llvm::SMRange range,
        unique_ptr<AstIdentExpr> ident,
        std::vector<unique_ptr<AstLiteralExpr>> args) noexcept
    : AstNode{ ID, range },
      identExpr{ std::move(ident) },
      argExprs{ std::move(args) } {};

    unique_ptr<AstIdentExpr> identExpr;
    std::vector<unique_ptr<AstLiteralExpr>> argExprs;
};

//----------------------------------------
// Declarations
//----------------------------------------
struct AstDecl : AstStmt {
    explicit AstDecl(
        AstKind kind,
        llvm::SMRange range,
        StringRef id_,
        unique_ptr<AstAttributeList> attribs) noexcept
    : AstStmt{kind, range},
      id{id_},
      attributes{std::move(attribs) } {}

    static constexpr bool classof(const AstRoot* ast) noexcept {
        return AST_DECL_RANGE(IS_AST_CLASSOF)
    }

    StringRef id;
    unique_ptr<AstAttributeList> attributes;
    Symbol* symbol = nullptr;
};

struct AstVarDecl final : AstNode<AstVarDecl, AstDecl, AstKind::VarDecl> {
    explicit AstVarDecl(
        llvm::SMRange range,
        StringRef id,
        unique_ptr<AstAttributeList> attrs,
        unique_ptr<AstTypeExpr> type,
        unique_ptr<AstExpr> expr_) noexcept
    : AstNode{ ID, range, id, std::move(attrs) },
      typeExpr{ std::move(type) },
      expr{ std::move(expr_) } {};

    unique_ptr<AstTypeExpr> typeExpr;
    unique_ptr<AstExpr> expr;
};

struct AstFuncDecl final : AstNode<AstFuncDecl, AstDecl, AstKind::FuncDecl> {
    explicit AstFuncDecl(
        llvm::SMRange range,
        StringRef id,
        unique_ptr<AstAttributeList> attrs,
        std::vector<unique_ptr<AstFuncParamDecl>> params,
        bool variadic_,
        unique_ptr<AstTypeExpr> retType_,
        bool hasImpl_) noexcept
    :  AstNode{ ID, range, id, std::move(attrs) },
      paramDecls{ std::move(params) },
      variadic{ variadic_ },
      retTypeExpr{ std::move(retType_) },
      hasImpl{ hasImpl_ } {};

    // declared parameters
    std::vector<unique_ptr<AstFuncParamDecl>> paramDecls;
    // is function variadic?
    bool variadic = false;
    // declared typeExpr
    unique_ptr<AstTypeExpr> retTypeExpr;
    // has implementation
    bool hasImpl = false;
    // scope symbol table for parameters
    unique_ptr<SymbolTable> symbolTable;
};

struct AstFuncParamDecl final : AstNode<AstFuncParamDecl, AstDecl, AstKind::FuncParamDecl> {
    explicit AstFuncParamDecl(
        llvm::SMRange range,
        StringRef id,
        unique_ptr<AstAttributeList> attrs,
        unique_ptr<AstTypeExpr> type) noexcept
    : AstNode{ ID, range, id, std::move(attrs) },
      typeExpr{ std::move(type) } {};

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
    explicit AstTypeExpr(
        llvm::SMRange range,
        TokenKind tokenKind_) noexcept
    : AstNode{ ID, range },
      tokenKind{ tokenKind_ } {};

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
    explicit AstIdentExpr(
        llvm::SMRange range,
        StringRef id_) noexcept
    : AstNode{ ID, range },
      id{ id_ } {};

    StringRef id;
    Symbol* symbol = nullptr;
};

struct AstCallExpr final : AstNode<AstCallExpr, AstExpr, AstKind::CallExpr> {
    explicit AstCallExpr(
        llvm::SMRange range,
        unique_ptr<AstIdentExpr> ident,
        std::vector<unique_ptr<AstExpr>> args) noexcept
    : AstNode{ ID, range },
      identExpr{ std::move(ident) },
      argExprs{ std::move(args) } {};

    unique_ptr<AstIdentExpr> identExpr;
    std::vector<unique_ptr<AstExpr>> argExprs;
};

struct AstLiteralExpr final : AstNode<AstLiteralExpr, AstExpr, AstKind::LiteralExpr> {
    using Value = Token::Value;

    explicit AstLiteralExpr(
        llvm::SMRange range,
        Value value_) noexcept
    : AstNode{ ID, range },
      value{ value_ } {};

    Value value{};
};

struct AstUnaryExpr final : AstNode<AstUnaryExpr, AstExpr, AstKind::UnaryExpr> {
    explicit AstUnaryExpr(
        llvm::SMRange range,
        TokenKind tokenKind_,
        unique_ptr<AstExpr> expr_) noexcept
    : AstNode{ ID, range },
      tokenKind{ tokenKind_ },
      expr{ std::move(expr_) } {};

    TokenKind tokenKind;
    unique_ptr<AstExpr> expr;
};

struct AstBinaryExpr final : AstNode<AstBinaryExpr, AstExpr, AstKind::BinaryExpr> {
    explicit AstBinaryExpr(
        llvm::SMRange range,
        TokenKind tokenKind_,
        unique_ptr<AstExpr> lhs_,
        unique_ptr<AstExpr> rhs_) noexcept
    : AstNode{ ID, range },
      tokenKind{ tokenKind_ },
      lhs{ std::move(lhs_) },
      rhs{ std::move(rhs_) } {};

    TokenKind tokenKind;
    unique_ptr<AstExpr> lhs;
    unique_ptr<AstExpr> rhs;
};

struct AstCastExpr final : AstNode<AstCastExpr, AstExpr, AstKind::CastExpr> {
    explicit AstCastExpr(
        llvm::SMRange range,
        unique_ptr<AstExpr> expr_,
        unique_ptr<AstTypeExpr> typeExpr_,
        bool implicit_) noexcept
    : AstNode{ ID, range },
      expr{ std::move(expr_) },
      typeExpr{ std::move(typeExpr_) },
      implicit{ implicit_ } {};

    unique_ptr<AstExpr> expr;
    unique_ptr<AstTypeExpr> typeExpr;
    bool implicit;
};

struct AstIfExpr final : AstNode<AstIfExpr, AstExpr, AstKind::IfExpr> {
    explicit AstIfExpr(
        llvm::SMRange range,
        unique_ptr<AstExpr> expr_,
        unique_ptr<AstExpr> trueExpr_,
        unique_ptr<AstExpr> falseExpr_) noexcept
    : AstNode{ ID, range },
      expr{ std::move(expr_) },
      trueExpr{ std::move(trueExpr_) },
      falseExpr{ std::move(falseExpr_) } {};

    unique_ptr<AstExpr> expr;
    unique_ptr<AstExpr> trueExpr;
    unique_ptr<AstExpr> falseExpr;
};

#undef IS_AST_CLASSOF

} // namespace lbc
