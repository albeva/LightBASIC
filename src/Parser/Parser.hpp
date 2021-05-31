//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "Ast/Ast.def.hpp"

namespace lbc {
class Context;
class Lexer;
class Token;
enum class TokenKind;
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
        CommaAsAnd = 1,
        AssignAsEqual = 2,
        Default = AssignAsEqual,
        LLVM_MARK_AS_BITMASK_ENUM(/* LargestValue = */ AssignAsEqual)
    };

    [[nodiscard]] unique_ptr<AstStmtList> stmtList();
    [[nodiscard]] unique_ptr<AstStmt> statement();
    [[nodiscard]] unique_ptr<AstStmt> declaration();

    [[nodiscard]] unique_ptr<AstExpr> expression(ExprFlags flags = ExprFlags::Default);
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

    [[nodiscard]] unique_ptr<AstAssignStmt> assignment(unique_ptr<AstIdentExpr> ident);
    [[nodiscard]] unique_ptr<AstExprStmt> callStmt(unique_ptr<AstIdentExpr> ident);
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

    // return true if has more content to parse
    [[nodiscard]] bool isValid() const noexcept;

    // match current token against kind
    [[nodiscard]] bool match(TokenKind kind) const noexcept;

    // replace token kind with another (e.g. Minus to Negate)
    void replace(TokenKind what, TokenKind with);

    // expect token to match, move to next token and return current
    // return nullptr otherwise
    [[nodiscard]] unique_ptr<Token> accept(TokenKind kind);

    // expect token to match, move to next token and return current
    // show error and terminate otherwise
    unique_ptr<Token> expect(TokenKind kind);

    // advance to the next token from the stream
    unique_ptr<Token> move();

    // show error and terminate compilation
    [[noreturn]] void error(const Twine& message);

    Context& m_context;
    unsigned m_fileId;
    bool m_isMain;
    Scope m_scope;
    unique_ptr<Lexer> m_lexer;
    unique_ptr<Token> m_token;
    unique_ptr<Token> m_next;
    llvm::SMLoc m_endLoc{};
    ExprFlags m_exprFlags{};
};

} // namespace lbc
