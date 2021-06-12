//
// Created by Albert Varaksin on 05/07/2020.
//
#pragma once
#include "Ast.def.hpp"
#include "ControlFlowStack.hpp"
#include "Lexer/Token.hpp"
#include "Symbol/SymbolTable.hpp"
#include "ValueFlags.hpp"

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
    constexpr AstRoot(AstKind kind_, llvm::SMRange range_) noexcept
    : kind{ kind_ }, range{ range_ } {}

    [[nodiscard]] StringRef getClassName() const noexcept;

    const AstKind kind;
    const llvm::SMRange range;

    // Make vanilla new/delete illegal.
    void* operator new(size_t) = delete;
    void operator delete(void*) = delete;

    // Allow placement new
    void* operator new(size_t /*size*/, void* ptr) {
        assert(ptr);
        return ptr;
    }
};

template<typename This, typename Base, AstKind kind>
struct AstNode : Base {
    using Base::Base;
    constexpr static AstKind KIND = kind;

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == kind;
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
        AstStmtList* stms) noexcept
    : AstNode{ KIND, range_ },
      fileId{ file },
      hasImplicitMain{ implicitMain },
      stmtList{ stms } {};

    const unsigned int fileId;
    const bool hasImplicitMain;
    AstStmtList* stmtList;
    SymbolTable* symbolTable;
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
        std::vector<AstStmt*> stmts_) noexcept
    : AstNode{ KIND, range_ },
      stmts{ std::move(stmts_) } {};

    std::vector<AstStmt*> stmts;
};

struct AstImport final : AstNode<AstImport, AstStmt, AstKind::Import> {
    AstImport(
        llvm::SMRange range_,
        StringRef import_,
        AstModule* module_ = nullptr) noexcept
    : AstNode{ KIND, range_ },
      import{ import_ },
      module{ module_ } {}

    const StringRef import;
    AstModule* module;
};

struct AstExprStmt final : AstNode<AstExprStmt, AstStmt, AstKind::ExprStmt> {
    AstExprStmt(
        llvm::SMRange range_,
        AstExpr* expr_) noexcept
    : AstNode{ KIND, range_ },
      expr{ expr_ } {};

    AstExpr* expr;
};

struct AstFuncStmt final : AstNode<AstFuncStmt, AstStmt, AstKind::FuncStmt> {
    AstFuncStmt(
        llvm::SMRange range_,
        AstFuncDecl* decl_,
        AstStmtList* stmtList_) noexcept
    : AstNode{ KIND, range_ },
      decl{ decl_ },
      stmtList{ stmtList_ } {};

    AstFuncDecl* decl;
    AstStmtList* stmtList;
};

struct AstReturnStmt final : AstNode<AstReturnStmt, AstStmt, AstKind::ReturnStmt> {
    AstReturnStmt(
        llvm::SMRange range_,
        AstExpr* expr_) noexcept
    : AstNode{ KIND, range_ },
      expr{ expr_ } {};

    AstExpr* expr;
};

struct AstIfStmtBlock final {
    std::vector<AstVarDecl*> decls;
    SymbolTable* symbolTable;
    AstExpr* expr;
    AstStmt* stmt;
};

struct AstIfStmt final : AstNode<AstIfStmt, AstStmt, AstKind::IfStmt> {
    AstIfStmt(
        llvm::SMRange range_,
        std::vector<AstIfStmtBlock> blocks_) noexcept
    : AstNode{ KIND, range_ },
      blocks{ std::move(blocks_) } {};

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
        std::vector<AstVarDecl*> decls_,
        AstVarDecl* iter_,
        AstExpr* limit_,
        AstExpr* step_,
        AstStmt* stmt_,
        StringRef next_) noexcept
    : AstNode{ KIND, range_ },
      decls{ std::move(decls_) },
      iterator{ iter_ },
      limit{ limit_ },
      step{ step_ },
      stmt{ stmt_ },
      next{ next_ } {};

    std::vector<AstVarDecl*> decls;
    AstVarDecl* iterator;
    AstExpr* limit;
    AstExpr* step;
    AstStmt* stmt;
    const StringRef next;

    Direction direction = Direction::Unknown;
    SymbolTable* symbolTable;
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
        std::vector<AstVarDecl*> decls_,
        Condition condition_,
        AstExpr* expr_,
        AstStmt* stmt_) noexcept
    : AstNode{ KIND, range_ },
      decls{ std::move(decls_) },
      condition{ condition_ },
      expr{ expr_ },
      stmt{ stmt_ } {}

    std::vector<AstVarDecl*> decls;
    const Condition condition;
    AstExpr* expr;
    AstStmt* stmt;
    SymbolTable* symbolTable;
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
        std::vector<AstAttribute*> attribs_) noexcept
    : AstNode{ KIND, range_ },
      attribs{ std::move(attribs_) } {};

    [[nodiscard]] bool exists(StringRef name) const noexcept;
    [[nodiscard]] std::optional<StringRef> getStringLiteral(StringRef key) const noexcept;

    std::vector<AstAttribute*> attribs;
};

struct AstAttribute final : AstNode<AstAttribute, AstAttr, AstKind::Attribute> {
    AstAttribute(
        llvm::SMRange range_,
        AstIdentExpr* ident,
        std::vector<AstLiteralExpr*> args) noexcept
    : AstNode{ KIND, range_ },
      identExpr{ ident },
      argExprs{ std::move(args) } {};

    AstIdentExpr* identExpr;
    std::vector<AstLiteralExpr*> argExprs;
};

//----------------------------------------
// Declarations
//----------------------------------------
struct AstDecl : AstStmt {
    AstDecl(
        AstKind kind_,
        llvm::SMRange range_,
        StringRef name_,
        AstAttributeList* attribs) noexcept
    : AstStmt{ kind_, range_ },
      name{ name_ },
      attributes{ attribs } {}

    static constexpr bool classof(const AstRoot* ast) noexcept {
        return AST_DECL_RANGE(IS_AST_CLASSOF)
    }

    const StringRef name;
    AstAttributeList* attributes;
    Symbol* symbol = nullptr;
};

struct AstVarDecl final : AstNode<AstVarDecl, AstDecl, AstKind::VarDecl> {
    AstVarDecl(
        llvm::SMRange range_,
        StringRef name_,
        AstAttributeList* attrs_,
        AstTypeExpr* type_,
        AstExpr* expr_) noexcept
    : AstNode{ KIND, range_, name_, attrs_ },
      typeExpr{ type_ },
      expr{ expr_ } {};

    AstTypeExpr* typeExpr;
    AstExpr* expr;
};

struct AstFuncDecl final : AstNode<AstFuncDecl, AstDecl, AstKind::FuncDecl> {
    AstFuncDecl(
        llvm::SMRange range_,
        StringRef name_,
        AstAttributeList* attrs_,
        std::vector<AstFuncParamDecl*> params_,
        bool variadic_,
        AstTypeExpr* retType_,
        bool hasImpl_) noexcept
    : AstNode{ KIND, range_, name_, attrs_ },
      paramDecls{ std::move(params_) },
      variadic{ variadic_ },
      retTypeExpr{ retType_ },
      hasImpl{ hasImpl_ } {};

    std::vector<AstFuncParamDecl*> paramDecls;
    const bool variadic;
    AstTypeExpr* retTypeExpr;
    const bool hasImpl;
    SymbolTable* symbolTable;
};

struct AstFuncParamDecl final : AstNode<AstFuncParamDecl, AstDecl, AstKind::FuncParamDecl> {
    AstFuncParamDecl(
        llvm::SMRange range_,
        StringRef name_,
        AstAttributeList* attrs,
        AstTypeExpr* type) noexcept
    : AstNode{ KIND, range_, name_, attrs },
      typeExpr{ type } {};

    AstTypeExpr* typeExpr;
};

struct AstTypeDecl final : AstNode<AstTypeDecl, AstDecl, AstKind::TypeDecl> {
    AstTypeDecl(
        llvm::SMRange range_,
        StringRef name_,
        AstAttributeList* attrs,
        std::vector<AstDecl*> decls_) noexcept
    : AstNode{ KIND, range_, name_, attrs },
      decls{ std::move(decls_) } {}

    std::vector<AstDecl*> decls;
    SymbolTable* symbolTable;
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
        AstIdentExpr* ident_,
        TokenKind tokenKind_,
        int deref) noexcept
    : AstNode{ KIND, range_ },
      ident{ ident_ },
      tokenKind{ tokenKind_ },
      dereference{ deref } {};

    // TODO use AstExpr?
    AstIdentExpr* ident;
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
    ValueFlags flags{};
};

struct AstAssignExpr final : AstNode<AstAssignExpr, AstExpr, AstKind::AssignExpr> {
    AstAssignExpr(
        llvm::SMRange range_,
        AstExpr* lhs_,
        AstExpr* rhs_) noexcept
    : AstNode{ KIND, range_ },
      lhs{ lhs_ },
      rhs{ rhs_ } {};

    AstExpr* lhs;
    AstExpr* rhs;
};

struct AstIdentExpr final : AstNode<AstIdentExpr, AstExpr, AstKind::IdentExpr> {
    AstIdentExpr(
        llvm::SMRange range_,
        StringRef name_) noexcept
    : AstNode{ KIND, range_ },
      name{ name_ } {};

    StringRef name;
    Symbol* symbol = nullptr;
};

struct AstCallExpr final : AstNode<AstCallExpr, AstExpr, AstKind::CallExpr> {
    AstCallExpr(
        llvm::SMRange range_,
        AstExpr* callable_,
        std::vector<AstExpr*> args_) noexcept
    : AstNode{ KIND, range_ },
      callable{ callable_ },
      args{ std::move(args_) } {};

    AstExpr* callable;
    std::vector<AstExpr*> args;
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
        AstExpr* expr_) noexcept
    : AstNode{ KIND, range_ },
      tokenKind{ tokenKind_ },
      expr{ expr_ } {};

    const TokenKind tokenKind;
    AstExpr* expr;
};

struct AstDereference final : AstNode<AstDereference, AstExpr, AstKind::Dereference> {
    AstDereference(
        llvm::SMRange range_,
        AstExpr* expr_) noexcept
    : AstNode{ KIND, range_ },
      expr{ expr_ } {};

    AstExpr* expr;
};

struct AstAddressOf final : AstNode<AstAddressOf, AstExpr, AstKind::AddressOf> {
    AstAddressOf(
        llvm::SMRange range_,
        AstExpr* expr_) noexcept
    : AstNode{ KIND, range_ },
      expr{ expr_ } {};

    AstExpr* expr;
};

struct AstMemberAccess final : AstNode<AstMemberAccess, AstExpr, AstKind::MemberAccess> {
    AstMemberAccess(
        llvm::SMRange range_,
        AstExpr* lhs_,
        AstExpr* rhs_) noexcept
    : AstNode{ KIND, range_ },
      lhs{ lhs_ },
      rhs{ rhs_ } {};

    AstExpr* lhs;
    AstExpr* rhs;
};

struct AstBinaryExpr final : AstNode<AstBinaryExpr, AstExpr, AstKind::BinaryExpr> {
    AstBinaryExpr(
        llvm::SMRange range_,
        TokenKind tokenKind_,
        AstExpr* lhs_,
        AstExpr* rhs_) noexcept
    : AstNode{ KIND, range_ },
      tokenKind{ tokenKind_ },
      lhs{ lhs_ },
      rhs{ rhs_ } {};

    const TokenKind tokenKind;
    AstExpr* lhs;
    AstExpr* rhs;
};

struct AstCastExpr final : AstNode<AstCastExpr, AstExpr, AstKind::CastExpr> {
    AstCastExpr(
        llvm::SMRange range_,
        AstExpr* expr_,
        AstTypeExpr* typeExpr_,
        bool implicit_) noexcept
    : AstNode{ KIND, range_ },
      expr{ expr_ },
      typeExpr{ typeExpr_ },
      implicit{ implicit_ } {};

    AstExpr* expr;
    AstTypeExpr* typeExpr;
    const bool implicit;
};

struct AstIfExpr final : AstNode<AstIfExpr, AstExpr, AstKind::IfExpr> {
    AstIfExpr(
        llvm::SMRange range_,
        AstExpr* expr_,
        AstExpr* trueExpr_,
        AstExpr* falseExpr_) noexcept
    : AstNode{ KIND, range_ },
      expr{ expr_ },
      trueExpr{ trueExpr_ },
      falseExpr{ falseExpr_ } {};

    AstExpr* expr;
    AstExpr* trueExpr;
    AstExpr* falseExpr;
};

#undef IS_AST_CLASSOF

} // namespace lbc
