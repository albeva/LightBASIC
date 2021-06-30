//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "Ast/Ast.def.hpp"
#include "Lexer/Token.hpp"

namespace lbc {
class Context;
class Lexer;
class DiagnosticEngine;
struct AstIfStmtBlock;
enum class Diag;
AST_FORWARD_DECLARE()

class Parser final {
public:
    NO_COPY_AND_MOVE(Parser)

    Parser(Context& context, unsigned int fileId, bool isMain);
    ~Parser() noexcept;

    [[nodiscard]] AstModule* parse();

private:
    enum class Scope {
        Root,
        Function
    };

    enum class ExprFlags : unsigned {
        None = 0,
        CommaAsAnd = 1,
        UseAssign = 2,
        CallWithoutParens = 4,
        LLVM_MARK_AS_BITMASK_ENUM(/* LargestValue = */ CallWithoutParens)
    };

    [[nodiscard]] bool stmtList(AstStmtList*&);
    [[nodiscard]] AstStmt* statement();
    [[nodiscard]] AstImport* kwImport();
    [[nodiscard]] AstStmt* declaration();
    [[nodiscard]] AstExpr* expression(ExprFlags flags = ExprFlags::None);
    [[nodiscard]] AstExpr* factor();
    [[nodiscard]] AstExpr* primary();
    [[nodiscard]] AstExpr* unary(llvm::SMRange range, TokenKind op, AstExpr* expr);
    [[nodiscard]] AstExpr* binary(llvm::SMRange range, TokenKind op, AstExpr* lhs, AstExpr* rhs);
    [[nodiscard]] AstExpr* expression(AstExpr* lhs, int precedence);
    [[nodiscard]] AstIdentExpr* identifier();
    [[nodiscard]] AstLiteralExpr* literal();
    [[nodiscard]] AstCallExpr* callExpr();
    [[nodiscard]] AstIfExpr* ifExpr();
    [[nodiscard]] AstExprList* expressionList();
    [[nodiscard]] AstVarDecl* kwVar(AstAttributeList* attribs);
    [[nodiscard]] AstIfStmt* kwIf();
    [[nodiscard]] AstIfStmtBlock ifBlock();
    [[nodiscard]] AstIfStmtBlock thenBlock(std::vector<AstVarDecl*> decls, AstExpr* expr);
    [[nodiscard]] AstForStmt* kwFor();
    [[nodiscard]] AstDoLoopStmt* kwDo();
    [[nodiscard]] AstContinuationStmt* kwContinue();
    [[nodiscard]] AstContinuationStmt* kwExit();
    [[nodiscard]] AstAttributeList* attributeList();
    [[nodiscard]] AstAttribute* attribute();
    [[nodiscard]] AstExprList* attributeArgList();
    [[nodiscard]] AstTypeExpr* typeExpr();
    [[nodiscard]] AstFuncDecl* kwDeclare(AstAttributeList* attribs);
    [[nodiscard]] AstFuncDecl* funcSignature(llvm::SMLoc start, AstAttributeList* attribs, bool hasImpl);
    [[nodiscard]] AstFuncParamList* funcParamList(bool& isVariadic);
    [[nodiscard]] AstFuncParamDecl* funcParam();
    [[nodiscard]] AstFuncStmt* kwFunction(AstAttributeList* attribs);
    [[nodiscard]] AstStmt* kwReturn();
    [[nodiscard]] AstTypeDecl* kwType(AstAttributeList* attribs);
    [[nodiscard]] AstDeclList* typeDeclList();
    [[nodiscard]] AstDecl* typeMember(AstAttributeList* attribs);

    // replace token kind with another (e.g. Minus to Negate)
    void replace(TokenKind what, TokenKind with) noexcept;

    // If token matches then advance and return true
    [[nodiscard]] bool accept(TokenKind kind) {
        if (m_token.is(kind)) {
            advance();
            return true;
        }
        return false;
    }

    // expects given token and advances.
    bool consume(TokenKind kind) noexcept {
        if (expect(kind)) {
            advance();
            return true;
        }
        return false;
    }

    // Expects a match, raises error when fails
    bool expect(TokenKind kind) noexcept;

    // advance to the next token from the stream
    void advance();

    Context& m_context;
    DiagnosticEngine& m_diag;
    const unsigned m_fileId;
    const bool m_isMain;
    Scope m_scope;
    unique_ptr<Lexer> m_lexer;
    Token m_token{};
    llvm::SMLoc m_endLoc{};
    ExprFlags m_exprFlags{};
};

} // namespace lbc
