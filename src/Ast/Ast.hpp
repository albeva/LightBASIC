//
// Created by Albert Varaksin on 05/07/2020.
//
#pragma once
#include "Ast.def.hpp"
#include "ControlFlowStack.hpp"
#include "Lexer/Token.hpp"
#include "Symbol/SymbolTable.hpp"
#include "ValueCategory.hpp"

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

    constexpr AstRoot(AstKind kind_, llvm::SMRange range_) noexcept
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
    constexpr static unique_ptr<This> create(Args&&... args) {
        return make_unique<This>(std::forward<Args>(args)...);
    }
};

#define IS_AST_CLASSOF(FIRST, LAST) ast->kind >= AstKind::FIRST && ast->kind <= AstKind::LAST;

//----------------------------------------
// Module
//----------------------------------------

struct AstModule final : AstNode<AstModule, AstRoot, AstKind::Module> {
    AstModule(
        unsigned int file,
        llvm::SMRange range_,
        bool implicitMain,
        unique_ptr<AstStmtList> stms) noexcept
    : AstNode{ KIND, range_ },
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
    AstStmtList(
        llvm::SMRange range_,
        std::vector<unique_ptr<AstStmt>> list) noexcept
    : AstNode{ KIND, range_ },
      stmts{ std::move(list) } {};

    std::vector<unique_ptr<AstStmt>> stmts;
};

struct AstExprStmt final : AstNode<AstExprStmt, AstStmt, AstKind::ExprStmt> {
    AstExprStmt(
        llvm::SMRange range_,
        unique_ptr<AstExpr> e) noexcept
    : AstNode{ KIND, range_ },
      expr{ std::move(e) } {};

    unique_ptr<AstExpr> expr;
};

struct AstFuncStmt final : AstNode<AstFuncStmt, AstStmt, AstKind::FuncStmt> {
    AstFuncStmt(
        llvm::SMRange range_,
        unique_ptr<AstFuncDecl> d,
        unique_ptr<AstStmtList> list) noexcept
    : AstNode{ KIND, range_ },
      decl{ std::move(d) },
      stmtList{ std::move(list) } {};

    unique_ptr<AstFuncDecl> decl;
    unique_ptr<AstStmtList> stmtList;
};

struct AstReturnStmt final : AstNode<AstReturnStmt, AstStmt, AstKind::ReturnStmt> {
    AstReturnStmt(
        llvm::SMRange range_,
        unique_ptr<AstExpr> e) noexcept
    : AstNode{ KIND, range_ },
      expr{ std::move(e) } {};

    unique_ptr<AstExpr> expr;
};

struct AstIfStmtBlock final {
    std::vector<unique_ptr<AstVarDecl>> decls;
    unique_ptr<SymbolTable> symbolTable;
    unique_ptr<AstExpr> expr;
    unique_ptr<AstStmt> stmt;
};

struct AstIfStmt final : AstNode<AstIfStmt, AstStmt, AstKind::IfStmt> {
    AstIfStmt(
        llvm::SMRange range_,
        std::vector<AstIfStmtBlock> list) noexcept
    : AstNode{ KIND, range_ },
      blocks{ std::move(list) } {};

    std::vector<AstIfStmtBlock> blocks;
};

struct AstForStmt final : AstNode<AstForStmt, AstStmt, AstKind::ForStmt> {
    enum class Direction {
        Unknown,
        Skip,
        Increment,
        Decrement
    };
    AstForStmt(
        llvm::SMRange range_,
        std::vector<unique_ptr<AstVarDecl>> decls_,
        unique_ptr<AstVarDecl> iter,
        unique_ptr<AstExpr> limit_,
        unique_ptr<AstExpr> step_,
        unique_ptr<AstStmt> stmt_,
        StringRef next_) noexcept
    : AstNode{ KIND, range_ },
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

    Direction direction = Direction::Unknown;
    unique_ptr<SymbolTable> symbolTable;
};

struct AstDoLoopStmt final : AstNode<AstDoLoopStmt, AstStmt, AstKind::DoLoopStmt> {
    enum class Condition {
        None,
        PreWhile,
        PreUntil,
        PostWhile,
        PostUntil
    };

    AstDoLoopStmt(
        llvm::SMRange range_,
        std::vector<unique_ptr<AstVarDecl>> decls_,
        Condition condition_,
        unique_ptr<AstExpr> expr_,
        unique_ptr<AstStmt> stmt_) noexcept
    : AstNode{ KIND, range_ },
      decls{ std::move(decls_) },
      condition{ condition_ },
      expr{ std::move(expr_) },
      stmt{ std::move(stmt_) } {}

    std::vector<unique_ptr<AstVarDecl>> decls;
    const Condition condition;
    unique_ptr<AstExpr> expr;
    unique_ptr<AstStmt> stmt;
    unique_ptr<SymbolTable> symbolTable;
};

struct AstControlFlowBranch final : AstNode<AstControlFlowBranch, AstStmt, AstKind::ControlFlowBranch> {
    enum class Action {
        Continue,
        Exit
    };

    explicit AstControlFlowBranch(
        llvm::SMRange range_,
        Action action_,
        std::vector<ControlFlowStatement> destination_) noexcept
    : AstNode{ KIND, range_ },
      action{ action_ },
      destination{ std::move(destination_) } {}

    Action action;
    std::vector<ControlFlowStatement> destination;
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
    AstAttributeList(
        llvm::SMRange range_,
        std::vector<unique_ptr<AstAttribute>> attribs_) noexcept
    : AstNode{ KIND, range_ },
      attribs{ std::move(attribs_) } {};

    [[nodiscard]] bool exists(StringRef name) const noexcept;
    [[nodiscard]] std::optional<StringRef> getStringLiteral(StringRef key) const noexcept;

    std::vector<unique_ptr<AstAttribute>> attribs;
};

struct AstAttribute final : AstNode<AstAttribute, AstAttr, AstKind::Attribute> {
    AstAttribute(
        llvm::SMRange range_,
        unique_ptr<AstIdentExpr> ident,
        std::vector<unique_ptr<AstLiteralExpr>> args) noexcept
    : AstNode{ KIND, range_ },
      identExpr{ std::move(ident) },
      argExprs{ std::move(args) } {};

    unique_ptr<AstIdentExpr> identExpr;
    std::vector<unique_ptr<AstLiteralExpr>> argExprs;
};

//----------------------------------------
// Declarations
//----------------------------------------
struct AstDecl : AstStmt {
    AstDecl(
        AstKind kind_,
        llvm::SMRange range_,
        StringRef name_,
        unique_ptr<AstAttributeList> attribs) noexcept
    : AstStmt{ kind_, range_ },
      name{ name_ },
      attributes{ std::move(attribs) } {}

    static constexpr bool classof(const AstRoot* ast) noexcept {
        return AST_DECL_RANGE(IS_AST_CLASSOF)
    }

    const StringRef name;
    unique_ptr<AstAttributeList> attributes;
    Symbol* symbol = nullptr;
};

struct AstVarDecl final : AstNode<AstVarDecl, AstDecl, AstKind::VarDecl> {
    AstVarDecl(
        llvm::SMRange range_,
        StringRef name_,
        unique_ptr<AstAttributeList> attrs,
        unique_ptr<AstTypeExpr> type,
        unique_ptr<AstExpr> expr_) noexcept
    : AstNode{ KIND, range_, name_, std::move(attrs) },
      typeExpr{ std::move(type) },
      expr{ std::move(expr_) } {};

    unique_ptr<AstTypeExpr> typeExpr;
    unique_ptr<AstExpr> expr;
};

struct AstFuncDecl final : AstNode<AstFuncDecl, AstDecl, AstKind::FuncDecl> {
    AstFuncDecl(
        llvm::SMRange range_,
        StringRef name_,
        unique_ptr<AstAttributeList> attrs,
        std::vector<unique_ptr<AstFuncParamDecl>> params,
        bool variadic_,
        unique_ptr<AstTypeExpr> retType_,
        bool hasImpl_) noexcept
    : AstNode{ KIND, range_, name_, std::move(attrs) },
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
    AstFuncParamDecl(
        llvm::SMRange range_,
        StringRef name_,
        unique_ptr<AstAttributeList> attrs,
        unique_ptr<AstTypeExpr> type) noexcept
    : AstNode{ KIND, range_, name_, std::move(attrs) },
      typeExpr{ std::move(type) } {};

    unique_ptr<AstTypeExpr> typeExpr;
};

struct AstTypeDecl final : AstNode<AstTypeDecl, AstDecl, AstKind::TypeDecl> {
    AstTypeDecl(
        llvm::SMRange range_,
        StringRef name_,
        unique_ptr<AstAttributeList> attrs,
        std::vector<unique_ptr<AstDecl>> decls_) noexcept
    : AstNode{ KIND, range_, name_, std::move(attrs) },
      decls{ std::move(decls_) } {}

    std::vector<unique_ptr<AstDecl>> decls;
    std::unique_ptr<SymbolTable> symbolTable;
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
    AstTypeExpr(
        llvm::SMRange range_,
        unique_ptr<AstIdentExpr> ident_,
        TokenKind tokenKind_,
        int deref) noexcept
    : AstNode{ KIND, range_ },
      ident{ std::move(ident_) },
      tokenKind{ tokenKind_ },
      dereference{ deref } {};

    // TODO use AstExpr?
    unique_ptr<AstIdentExpr> ident;
    const TokenKind tokenKind;
    const int dereference;
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
    ValueCategory category{ ValueCategory::None };
};

struct AstAssignExpr final : AstNode<AstAssignExpr, AstExpr, AstKind::AssignExpr> {
    AstAssignExpr(
        llvm::SMRange range_,
        unique_ptr<AstExpr> lhs_,
        unique_ptr<AstExpr> rhs_) noexcept
    : AstNode{ KIND, range_ },
      lhs{ std::move(lhs_) },
      rhs{ std::move(rhs_) } {};

    unique_ptr<AstExpr> lhs, rhs;
};

struct AstIdentExpr final : AstNode<AstIdentExpr, AstExpr, AstKind::IdentExpr> {
    AstIdentExpr(
        llvm::SMRange range_,
        StringRef name_) noexcept
    : AstNode{ KIND, range_ },
      name{ name_ } {};

    StringRef name;
    Symbol* symbol;
};

struct AstCallExpr final : AstNode<AstCallExpr, AstExpr, AstKind::CallExpr> {
    AstCallExpr(
        llvm::SMRange range_,
        unique_ptr<AstExpr> callable_,
        std::vector<unique_ptr<AstExpr>> args_) noexcept
    : AstNode{ KIND, range_ },
      callable{ std::move(callable_) },
      args{ std::move(args_) } {};

    unique_ptr<AstExpr> callable;
    std::vector<unique_ptr<AstExpr>> args;
};

struct AstLiteralExpr final : AstNode<AstLiteralExpr, AstExpr, AstKind::LiteralExpr> {
    using Value = Token::Value;

    AstLiteralExpr(
        llvm::SMRange range_,
        Value value_) noexcept
    : AstNode{ KIND, range_ },
      value{ value_ } {};

    const Value value;
};

struct AstUnaryExpr final : AstNode<AstUnaryExpr, AstExpr, AstKind::UnaryExpr> {
    AstUnaryExpr(
        llvm::SMRange range_,
        TokenKind tokenKind_,
        unique_ptr<AstExpr> expr_) noexcept
    : AstNode{ KIND, range_ },
      tokenKind{ tokenKind_ },
      expr{ std::move(expr_) } {};

    const TokenKind tokenKind;
    unique_ptr<AstExpr> expr;
};

struct AstDereference final : AstNode<AstDereference, AstExpr, AstKind::Dereference> {
    AstDereference(
        llvm::SMRange range_,
        unique_ptr<AstExpr> expr_) noexcept
    : AstNode{ KIND, range_ },
      expr{ std::move(expr_) } {};

    unique_ptr<AstExpr> expr;
};

struct AstAddressOf final : AstNode<AstAddressOf, AstExpr, AstKind::AddressOf> {
    AstAddressOf(
        llvm::SMRange range_,
        unique_ptr<AstExpr> expr_) noexcept
    : AstNode{ KIND, range_ },
      expr{ std::move(expr_) } {};

    unique_ptr<AstExpr> expr;
};

struct AstMemberAccess final : AstNode<AstMemberAccess, AstExpr, AstKind::MemberAccess> {
    AstMemberAccess(
        llvm::SMRange range_,
        unique_ptr<AstExpr> lhs_,
        unique_ptr<AstExpr> rhs_) noexcept
    : AstNode{ KIND, range_ },
      lhs{ std::move(lhs_) },
      rhs{ std::move(rhs_) } {};

    unique_ptr<AstExpr> lhs;
    unique_ptr<AstExpr> rhs;
};

struct AstBinaryExpr final : AstNode<AstBinaryExpr, AstExpr, AstKind::BinaryExpr> {
    AstBinaryExpr(
        llvm::SMRange range_,
        TokenKind tokenKind_,
        unique_ptr<AstExpr> lhs_,
        unique_ptr<AstExpr> rhs_) noexcept
    : AstNode{ KIND, range_ },
      tokenKind{ tokenKind_ },
      lhs{ std::move(lhs_) },
      rhs{ std::move(rhs_) } {};

    const TokenKind tokenKind;
    unique_ptr<AstExpr> lhs;
    unique_ptr<AstExpr> rhs;
};

struct AstCastExpr final : AstNode<AstCastExpr, AstExpr, AstKind::CastExpr> {
    AstCastExpr(
        llvm::SMRange range_,
        unique_ptr<AstExpr> expr_,
        unique_ptr<AstTypeExpr> typeExpr_,
        bool implicit_) noexcept
    : AstNode{ KIND, range_ },
      expr{ std::move(expr_) },
      typeExpr{ std::move(typeExpr_) },
      implicit{ implicit_ } {};

    unique_ptr<AstExpr> expr;
    unique_ptr<AstTypeExpr> typeExpr;
    const bool implicit;
};

struct AstIfExpr final : AstNode<AstIfExpr, AstExpr, AstKind::IfExpr> {
    AstIfExpr(
        llvm::SMRange range_,
        unique_ptr<AstExpr> expr_,
        unique_ptr<AstExpr> trueExpr_,
        unique_ptr<AstExpr> falseExpr_) noexcept
    : AstNode{ KIND, range_ },
      expr{ std::move(expr_) }, trueExpr{ std::move(trueExpr_) }, falseExpr{ std::move(falseExpr_) } {};

    unique_ptr<AstExpr> expr;
    unique_ptr<AstExpr> trueExpr;
    unique_ptr<AstExpr> falseExpr;
};

#undef IS_AST_CLASSOF

} // namespace lbc
