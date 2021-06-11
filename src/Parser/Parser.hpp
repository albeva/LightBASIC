//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "Ast/Ast.def.hpp"
#include "Lexer/Token.hpp"

namespace lbc {
class Context;
class Lexer;
struct AstIfStmtBlock;
AST_FORWARD_DECLARE()

class Parser final {
public:
    NO_COPY_AND_MOVE(Parser)

    Parser(Context& context, unsigned int fileId, bool isMain);
    ~Parser() noexcept;

    [[nodiscard]] unique_ptr<AstModule> parse();

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

    [[nodiscard]] unique_ptr<AstStmtList> stmtList();
    [[nodiscard]] unique_ptr<AstStmt> statement();
    [[nodiscard]] unique_ptr<AstStmtList> kwImport();
    [[nodiscard]] unique_ptr<AstStmt> declaration();

    [[nodiscard]] unique_ptr<AstExpr> expression(ExprFlags flags = ExprFlags::None);
    [[nodiscard]] unique_ptr<AstExpr> factor();
    [[nodiscard]] unique_ptr<AstExpr> primary();
    [[nodiscard]] unique_ptr<AstExpr> unary(llvm::SMRange range, TokenKind op, unique_ptr<AstExpr> expr);
    [[nodiscard]] unique_ptr<AstExpr> binary(llvm::SMRange range, TokenKind op, unique_ptr<AstExpr> lhs, unique_ptr<AstExpr> rhs);
    [[nodiscard]] unique_ptr<AstExpr> expression(unique_ptr<AstExpr> lhs, int precedence);
    [[nodiscard]] unique_ptr<AstIdentExpr> identifier();
    [[nodiscard]] unique_ptr<AstLiteralExpr> literal();
    [[nodiscard]] unique_ptr<AstCallExpr> callExpr();
    [[nodiscard]] unique_ptr<AstIfExpr> ifExpr();
    [[nodiscard]] std::vector<unique_ptr<AstExpr>> expressionList();

    [[nodiscard]] unique_ptr<AstVarDecl> kwVar(unique_ptr<AstAttributeList> attribs);
    [[nodiscard]] unique_ptr<AstIfStmt> kwIf();
    [[nodiscard]] AstIfStmtBlock ifBlock();
    [[nodiscard]] AstIfStmtBlock thenBlock(std::vector<unique_ptr<AstVarDecl>> decls, unique_ptr<AstExpr> expr);
    [[nodiscard]] unique_ptr<AstForStmt> kwFor();
    [[nodiscard]] unique_ptr<AstDoLoopStmt> kwDo();
    [[nodiscard]] unique_ptr<AstControlFlowBranch> kwContinue();
    [[nodiscard]] unique_ptr<AstControlFlowBranch> kwExit();

    [[nodiscard]] unique_ptr<AstAttributeList> attributeList();
    [[nodiscard]] unique_ptr<AstAttribute> attribute();
    [[nodiscard]] std::vector<unique_ptr<AstLiteralExpr>> attributeArgList();

    [[nodiscard]] unique_ptr<AstTypeExpr> typeExpr();

    [[nodiscard]] unique_ptr<AstFuncDecl> kwDeclare(unique_ptr<AstAttributeList> attribs);
    [[nodiscard]] unique_ptr<AstFuncDecl> funcSignature(llvm::SMLoc start, unique_ptr<AstAttributeList> attribs, bool hasImpl);
    [[nodiscard]] std::vector<unique_ptr<AstFuncParamDecl>> funcParamList(bool& isVariadic);
    [[nodiscard]] unique_ptr<AstFuncParamDecl> funcParam();
    [[nodiscard]] unique_ptr<AstFuncStmt> kwFunction(unique_ptr<AstAttributeList> attribs);
    [[nodiscard]] unique_ptr<AstStmt> kwReturn();

    [[nodiscard]] unique_ptr<AstTypeDecl> kwType(unique_ptr<AstAttributeList> attribs);
    [[nodiscard]] std::vector<unique_ptr<AstDecl>> typeDeclList();
    [[nodiscard]] unique_ptr<AstDecl> typeMember(unique_ptr<AstAttributeList> attribs);

    // replace token kind with another (e.g. Minus to Negate)
    void replace(TokenKind what, TokenKind with) noexcept;

    // return true if has more content to parse
    [[nodiscard]] bool isValid() const noexcept {
        return !match(TokenKind::EndOfFile);
    }

    // match current token against kind
    [[nodiscard]] bool match(TokenKind kind) const noexcept {
        return m_token.is(kind);
    }

    // expect token to match and if true, advances
    [[nodiscard]] bool accept(TokenKind kind);

    // Expects a match, raises error when fails
    bool expect(TokenKind kind) noexcept;

    // advance to the next token from the stream
    void advance();

    // show error and terminate compilation
    [[noreturn]] void error(const Twine& message);

    Context& m_context;
    const unsigned m_fileId;
    const bool m_isMain;
    Scope m_scope;
    unique_ptr<Lexer> m_lexer;
    Token m_token{};
    llvm::SMLoc m_endLoc{};
    ExprFlags m_exprFlags{};
};

} // namespace lbc
