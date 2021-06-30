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

#define IS_AST_CLASSOF(FIRST, LAST) ast->kind >= AstKind::FIRST && ast->kind <= AstKind::LAST;

//----------------------------------------
// Module
//----------------------------------------

struct AstModule final : AstRoot {
    AstModule(
        unsigned int file,
        llvm::SMRange range_,
        bool implicitMain,
        AstStmtList* stms) noexcept
    : AstRoot{ AstKind::Module, range_ },
      fileId{ file },
      hasImplicitMain{ implicitMain },
      stmtList{ stms } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::Module;
    }

    const unsigned int fileId;
    const bool hasImplicitMain;
    AstStmtList* stmtList;
    SymbolTable* symbolTable = nullptr;
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

struct AstStmtList final : AstStmt {
    AstStmtList(
        llvm::SMRange range_,
        std::vector<AstStmt*> stmts_) noexcept
    : AstStmt{ AstKind::StmtList, range_ },
      stmts{ std::move(stmts_) } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::StmtList;
    }

    std::vector<AstStmt*> stmts;
};

struct AstImport final : AstStmt {
    AstImport(
        llvm::SMRange range_,
        StringRef import_,
        AstModule* module_ = nullptr) noexcept
    : AstStmt{ AstKind::Import, range_ },
      import{ import_ },
      module{ module_ } {}

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::Import;
    }

    const StringRef import;
    AstModule* module;
};

struct AstExprStmt final : AstStmt {
    AstExprStmt(
        llvm::SMRange range_,
        AstExpr* expr_) noexcept
    : AstStmt{ AstKind::ExprStmt, range_ },
      expr{ expr_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::ExprStmt;
    }

    AstExpr* expr;
};

struct AstFuncStmt final : AstStmt {
    AstFuncStmt(
        llvm::SMRange range_,
        AstFuncDecl* decl_,
        AstStmtList* stmtList_) noexcept
    : AstStmt{ AstKind::FuncStmt, range_ },
      decl{ decl_ },
      stmtList{ stmtList_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::FuncStmt;
    }

    AstFuncDecl* decl;
    AstStmtList* stmtList;
};

struct AstReturnStmt final : AstStmt {
    AstReturnStmt(
        llvm::SMRange range_,
        AstExpr* expr_) noexcept
    : AstStmt{ AstKind::ReturnStmt, range_ },
      expr{ expr_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::ReturnStmt;
    }

    AstExpr* expr;
};

struct AstIfStmtBlock final {
    std::vector<AstVarDecl*> decls;
    SymbolTable* symbolTable;
    AstExpr* expr;
    AstStmt* stmt;
};

struct AstIfStmt final : AstStmt {
    AstIfStmt(
        llvm::SMRange range_,
        std::vector<AstIfStmtBlock> blocks_) noexcept
    : AstStmt{ AstKind::IfStmt, range_ },
      blocks{ std::move(blocks_) } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::IfStmt;
    }

    std::vector<AstIfStmtBlock> blocks;
};

struct AstForStmt final : AstStmt {
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
    : AstStmt{ AstKind::ForStmt, range_ },
      decls{ std::move(decls_) },
      iterator{ iter_ },
      limit{ limit_ },
      step{ step_ },
      stmt{ stmt_ },
      next{ next_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::ForStmt;
    }

    std::vector<AstVarDecl*> decls;
    AstVarDecl* iterator;
    AstExpr* limit;
    AstExpr* step;
    AstStmt* stmt;
    const StringRef next;

    Direction direction = Direction::Unknown;
    SymbolTable* symbolTable = nullptr;
};

struct AstDoLoopStmt final : AstStmt {
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
    : AstStmt{ AstKind::DoLoopStmt, range_ },
      decls{ std::move(decls_) },
      condition{ condition_ },
      expr{ expr_ },
      stmt{ stmt_ } {}

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::DoLoopStmt;
    }

    std::vector<AstVarDecl*> decls;
    const Condition condition;
    AstExpr* expr;
    AstStmt* stmt;
    SymbolTable* symbolTable = nullptr;
};

struct AstContinuationStmt final : AstStmt {
    enum class Action {
        Continue,
        Exit
    };

    explicit AstContinuationStmt(
        llvm::SMRange range_,
        Action action_,
        std::vector<ControlFlowStatement> destination_) noexcept
    : AstStmt{ AstKind::ContinuationStmt, range_ },
      action{ action_ },
      destination{ std::move(destination_) } {}

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::ContinuationStmt;
    }

    Action action;
    std::vector<ControlFlowStatement> destination;
};

//----------------------------------------
// Attributes
//----------------------------------------

struct AstAttributeList final : AstRoot {
    AstAttributeList(
        llvm::SMRange range_,
        std::vector<AstAttribute*> attribs_) noexcept
    : AstRoot{ AstKind::AttributeList, range_ },
      attribs{ std::move(attribs_) } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::AttributeList;
    }

    [[nodiscard]] bool exists(StringRef name) const noexcept;
    [[nodiscard]] std::optional<StringRef> getStringLiteral(StringRef key) const noexcept;

    std::vector<AstAttribute*> attribs;
};

struct AstAttribute final : AstRoot {
    AstAttribute(
        llvm::SMRange range_,
        AstIdentExpr* ident,
        AstExprList* args_) noexcept
    : AstRoot{ AstKind::Attribute, range_ },
      identExpr{ ident },
      args{ args_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::Attribute;
    }

    AstIdentExpr* identExpr;
    AstExprList* args;
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

struct AstDeclList final : AstRoot {
    AstDeclList(llvm::SMRange range_, std::vector<AstDecl*> decls_) noexcept
    : AstRoot{ AstKind::DeclList, range_ },
      decls{ std::move(decls_) } {}

    std::vector<AstDecl*> decls;
};

struct AstVarDecl final : AstDecl {
    AstVarDecl(
        llvm::SMRange range_,
        StringRef name_,
        AstAttributeList* attrs_,
        AstTypeExpr* type_,
        AstExpr* expr_) noexcept
    : AstDecl{ AstKind::VarDecl, range_, name_, attrs_ },
      typeExpr{ type_ },
      expr{ expr_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::VarDecl;
    }

    AstTypeExpr* typeExpr;
    AstExpr* expr;
};

struct AstFuncDecl final : AstDecl {
    AstFuncDecl(
        llvm::SMRange range_,
        StringRef name_,
        AstAttributeList* attrs_,
        AstFuncParamList* params_,
        bool variadic_,
        AstTypeExpr* retType_,
        bool hasImpl_) noexcept
    : AstDecl{ AstKind::FuncDecl, range_, name_, attrs_ },
      params{ params_ },
      variadic{ variadic_ },
      retTypeExpr{ retType_ },
      hasImpl{ hasImpl_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::FuncDecl;
    }

    AstFuncParamList* params;
    const bool variadic;
    AstTypeExpr* retTypeExpr;
    const bool hasImpl;
    SymbolTable* symbolTable = nullptr;
};

struct AstFuncParamDecl final : AstDecl {
    AstFuncParamDecl(
        llvm::SMRange range_,
        StringRef name_,
        AstAttributeList* attrs,
        AstTypeExpr* type) noexcept
    : AstDecl{ AstKind::FuncParamDecl, range_, name_, attrs },
      typeExpr{ type } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::FuncParamDecl;
    }

    AstTypeExpr* typeExpr;
};

struct AstFuncParamList final : AstRoot {
    AstFuncParamList(
        llvm::SMRange range_,
        std::vector<AstFuncParamDecl*> params_) noexcept
    : AstRoot{ AstKind::FuncParamList, range_ },
      params{ std::move(params_) } {}

    std::vector<AstFuncParamDecl*> params;
};

struct AstTypeDecl final : AstDecl {
    AstTypeDecl(
        llvm::SMRange range_,
        StringRef name_,
        AstAttributeList* attrs,
        AstDeclList* decls_) noexcept
    : AstDecl{ AstKind::TypeDecl, range_, name_, attrs },
      decls{ decls_ } {}

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::TypeDecl;
    }

    AstDeclList* decls;
    SymbolTable* symbolTable = nullptr;
};

//----------------------------------------
// Types
//----------------------------------------
struct AstTypeExpr final : AstRoot {
    AstTypeExpr(
        llvm::SMRange range_,
        AstIdentExpr* ident_,
        TokenKind tokenKind_,
        int deref) noexcept
    : AstRoot{ AstKind::TypeExpr, range_ },
      ident{ ident_ },
      tokenKind{ tokenKind_ },
      dereference{ deref } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::TypeExpr;
    }

    // TODO use AstExpr?
    AstIdentExpr* ident;
    const TokenKind tokenKind;
    const int dereference;
    const TypeRoot* type = nullptr;
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

struct AstExprList : AstRoot {
    AstExprList(
        llvm::SMRange range_,
        std::vector<AstExpr*> exprs_) noexcept
    : AstRoot{ AstKind::ExprList, range_ },
      exprs{ std::move(exprs_) } {}

    std::vector<AstExpr*> exprs;
};

struct AstAssignExpr final : AstExpr {
    AstAssignExpr(
        llvm::SMRange range_,
        AstExpr* lhs_,
        AstExpr* rhs_) noexcept
    : AstExpr{ AstKind::AssignExpr, range_ },
      lhs{ lhs_ },
      rhs{ rhs_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::AssignExpr;
    }

    AstExpr* lhs;
    AstExpr* rhs;
};

struct AstIdentExpr final : AstExpr {
    AstIdentExpr(
        llvm::SMRange range_,
        StringRef name_) noexcept
    : AstExpr{ AstKind::IdentExpr, range_ },
      name{ name_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::IdentExpr;
    }

    StringRef name;
    Symbol* symbol = nullptr;
};

struct AstCallExpr final : AstExpr {
    AstCallExpr(
        llvm::SMRange range_,
        AstExpr* callable_,
        AstExprList* args_) noexcept
    : AstExpr{ AstKind::CallExpr, range_ },
      callable{ callable_ },
      args{ args_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::CallExpr;
    }

    AstExpr* callable;
    AstExprList* args;
};

struct AstLiteralExpr final : AstExpr {
    using Value = Token::Value;

    AstLiteralExpr(
        llvm::SMRange range_,
        Value value_) noexcept
    : AstExpr{ AstKind::LiteralExpr, range_ },
      value{ value_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::LiteralExpr;
    }

    const Value value;
};

struct AstUnaryExpr final : AstExpr {
    AstUnaryExpr(
        llvm::SMRange range_,
        TokenKind tokenKind_,
        AstExpr* expr_) noexcept
    : AstExpr{ AstKind::UnaryExpr, range_ },
      tokenKind{ tokenKind_ },
      expr{ expr_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::UnaryExpr;
    }

    const TokenKind tokenKind;
    AstExpr* expr;
};

struct AstDereference final : AstExpr {
    AstDereference(
        llvm::SMRange range_,
        AstExpr* expr_) noexcept
    : AstExpr{ AstKind::Dereference, range_ },
      expr{ expr_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::Dereference;
    }

    AstExpr* expr;
};

struct AstAddressOf final : AstExpr {
    AstAddressOf(
        llvm::SMRange range_,
        AstExpr* expr_) noexcept
    : AstExpr{ AstKind::AddressOf, range_ },
      expr{ expr_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::AddressOf;
    }

    AstExpr* expr;
};

struct AstMemberAccess final : AstExpr {
    AstMemberAccess(
        llvm::SMRange range_,
        AstExpr* lhs_,
        AstExpr* rhs_) noexcept
    : AstExpr{ AstKind::MemberAccess, range_ },
      lhs{ lhs_ },
      rhs{ rhs_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::MemberAccess;
    }

    AstExpr* lhs;
    AstExpr* rhs;
};

struct AstBinaryExpr final : AstExpr {
    AstBinaryExpr(
        llvm::SMRange range_,
        TokenKind tokenKind_,
        AstExpr* lhs_,
        AstExpr* rhs_) noexcept
    : AstExpr{ AstKind::BinaryExpr, range_ },
      tokenKind{ tokenKind_ },
      lhs{ lhs_ },
      rhs{ rhs_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::BinaryExpr;
    }

    const TokenKind tokenKind;
    AstExpr* lhs;
    AstExpr* rhs;
};

struct AstCastExpr final : AstExpr {
    AstCastExpr(
        llvm::SMRange range_,
        AstExpr* expr_,
        AstTypeExpr* typeExpr_,
        bool implicit_) noexcept
    : AstExpr{ AstKind::CastExpr, range_ },
      expr{ expr_ },
      typeExpr{ typeExpr_ },
      implicit{ implicit_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::CastExpr;
    }

    AstExpr* expr;
    AstTypeExpr* typeExpr;
    const bool implicit;
};

struct AstIfExpr final : AstExpr {
    AstIfExpr(
        llvm::SMRange range_,
        AstExpr* expr_,
        AstExpr* trueExpr_,
        AstExpr* falseExpr_) noexcept
    : AstExpr{ AstKind::IfExpr, range_ },
      expr{ expr_ },
      trueExpr{ trueExpr_ },
      falseExpr{ falseExpr_ } {};

    constexpr static bool classof(const AstRoot* ast) noexcept {
        return ast->kind == AstKind::IfExpr;
    }

    AstExpr* expr;
    AstExpr* trueExpr;
    AstExpr* falseExpr;
};

#undef IS_AST_CLASSOF

} // namespace lbc
