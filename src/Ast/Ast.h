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

    constexpr explicit AstRoot(AstKind kind_, llvm::SMRange range_) noexcept
    : kind{ kind_ }, range{ range_ } {}

    virtual ~AstRoot() noexcept = default;

    [[nodiscard]] StringRef getClassName() const noexcept;

    const AstKind kind;
    const llvm::SMRange range;
};

template<typename This, typename Base, AstKind kind>
struct AstNode : Base {
    using Base::Base;
    constexpr static AstKind KIND = kind;

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == kind;
    }

    template<typename... Args>
    constexpr static unique_ptr<This> create(Args&&... args) noexcept {
        return make_unique<This>(std::forward<Args>(args)...);
    }
};

#define IS_AST_CLASSOF(FIRST, LAST) ast->kind >= AstKind::FIRST && ast->kind <= AstKind::LAST;

//----------------------------------------
// Module
//----------------------------------------

struct AstModule final : AstNode<AstModule, AstRoot, AstKind::Module> {
    explicit AstModule(
        unsigned int file,
        llvm::SMRange range,
        bool implicitMain,
        unique_ptr<AstStmtList> stms) noexcept
    : AstNode{ KIND, range },
      fileId{ file },
      hasImplicitMain{ implicitMain },
      stmtList{ std::move(stms) } {};

    const unsigned int fileId;
    const bool hasImplicitMain;
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
    : AstNode{ KIND, range },
      stmts{ std::move(list) } {};

    std::vector<unique_ptr<AstStmt>> stmts;
};

struct AstExprStmt final : AstNode<AstExprStmt, AstStmt, AstKind::ExprStmt> {
    explicit AstExprStmt(
        llvm::SMRange range,
        unique_ptr<AstExpr> e) noexcept
    : AstNode{ KIND, range },
      expr{ std::move(e) } {};

    unique_ptr<AstExpr> expr;
};

struct AstAssignStmt final : AstNode<AstAssignStmt, AstStmt, AstKind::AssignStmt> {
    explicit AstAssignStmt(
        llvm::SMRange range,
        unique_ptr<AstIdentExpr> ident,
        unique_ptr<AstExpr> e) noexcept
    : AstNode{ KIND, range },
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
    : AstNode{ KIND, range },
      decl{ std::move(d) },
      stmtList{ std::move(list) } {};

    unique_ptr<AstFuncDecl> decl;
    unique_ptr<AstStmtList> stmtList;
};

struct AstReturnStmt final : AstNode<AstReturnStmt, AstStmt, AstKind::ReturnStmt> {
    explicit AstReturnStmt(
        llvm::SMRange range,
        unique_ptr<AstExpr> e) noexcept
    : AstNode{ KIND, range },
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
    : AstNode{ KIND, range },
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
    : AstNode{ KIND, range },
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
    const StringRef next;
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
    : AstNode{ KIND, range },
      attribs{ std::move(attribs_) } {};

    [[nodiscard]] std::optional<StringRef> getStringLiteral(StringRef key) const;

    std::vector<unique_ptr<AstAttribute>> attribs;
};

struct AstAttribute final : AstNode<AstAttribute, AstAttr, AstKind::Attribute> {
    explicit AstAttribute(
        llvm::SMRange range,
        unique_ptr<AstIdentExpr> ident,
        std::vector<unique_ptr<AstLiteralExpr>> args) noexcept
    : AstNode{ KIND, range },
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
    : AstStmt{ kind, range },
      id{ id_ },
      attributes{ std::move(attribs) } {}

    static constexpr bool classof(const AstRoot* ast) noexcept {
        return AST_DECL_RANGE(IS_AST_CLASSOF)
    }

    const StringRef id;
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
    : AstNode{ KIND, range, id, std::move(attrs) },
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
    : AstNode{ KIND, range, id, std::move(attrs) },
      paramDecls{ std::move(params) },
      variadic{ variadic_ },
      retTypeExpr{ std::move(retType_) },
      hasImpl{ hasImpl_ } {};

    std::vector<unique_ptr<AstFuncParamDecl>> paramDecls;
    const bool variadic;
    unique_ptr<AstTypeExpr> retTypeExpr;
    const bool hasImpl;
    unique_ptr<SymbolTable> symbolTable;
};

struct AstFuncParamDecl final : AstNode<AstFuncParamDecl, AstDecl, AstKind::FuncParamDecl> {
    explicit AstFuncParamDecl(
        llvm::SMRange range,
        StringRef id,
        unique_ptr<AstAttributeList> attrs,
        unique_ptr<AstTypeExpr> type) noexcept
    : AstNode{ KIND, range, id, std::move(attrs) },
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
    : AstNode{ KIND, range },
      tokenKind{ tokenKind_ } {};

    const TokenKind tokenKind;
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
    : AstNode{ KIND, range },
      id{ id_ } {};

    const StringRef id;
    Symbol* symbol = nullptr;
};

struct AstCallExpr final : AstNode<AstCallExpr, AstExpr, AstKind::CallExpr> {
    explicit AstCallExpr(
        llvm::SMRange range,
        unique_ptr<AstIdentExpr> ident,
        std::vector<unique_ptr<AstExpr>> args) noexcept
    : AstNode{ KIND, range },
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
    : AstNode{ KIND, range },
      value{ value_ } {};

    const Value value;
};

struct AstUnaryExpr final : AstNode<AstUnaryExpr, AstExpr, AstKind::UnaryExpr> {
    explicit AstUnaryExpr(
        llvm::SMRange range,
        TokenKind tokenKind_,
        unique_ptr<AstExpr> expr_) noexcept
    : AstNode{ KIND, range },
      tokenKind{ tokenKind_ },
      expr{ std::move(expr_) } {};

    const TokenKind tokenKind;
    unique_ptr<AstExpr> expr;
};

struct AstBinaryExpr final : AstNode<AstBinaryExpr, AstExpr, AstKind::BinaryExpr> {
    explicit AstBinaryExpr(
        llvm::SMRange range,
        TokenKind tokenKind_,
        unique_ptr<AstExpr> lhs_,
        unique_ptr<AstExpr> rhs_) noexcept
    : AstNode{ KIND, range },
      tokenKind{ tokenKind_ },
      lhs{ std::move(lhs_) },
      rhs{ std::move(rhs_) } {};

    const TokenKind tokenKind;
    unique_ptr<AstExpr> lhs;
    unique_ptr<AstExpr> rhs;
};

struct AstCastExpr final : AstNode<AstCastExpr, AstExpr, AstKind::CastExpr> {
    explicit AstCastExpr(
        llvm::SMRange range,
        unique_ptr<AstExpr> expr_,
        unique_ptr<AstTypeExpr> typeExpr_,
        bool implicit_) noexcept
    : AstNode{ KIND, range },
      expr{ std::move(expr_) },
      typeExpr{ std::move(typeExpr_) },
      implicit{ implicit_ } {};

    unique_ptr<AstExpr> expr;
    unique_ptr<AstTypeExpr> typeExpr;
    const bool implicit;
};

struct AstIfExpr final : AstNode<AstIfExpr, AstExpr, AstKind::IfExpr> {
    explicit AstIfExpr(
        llvm::SMRange range,
        unique_ptr<AstExpr> expr_,
        unique_ptr<AstExpr> trueExpr_,
        unique_ptr<AstExpr> falseExpr_) noexcept
    : AstNode{ KIND, range },
      expr{ std::move(expr_) },
      trueExpr{ std::move(trueExpr_) },
      falseExpr{ std::move(falseExpr_) } {};

    unique_ptr<AstExpr> expr;
    unique_ptr<AstExpr> trueExpr;
    unique_ptr<AstExpr> falseExpr;
};

#undef IS_AST_CLASSOF

} // namespace lbc
