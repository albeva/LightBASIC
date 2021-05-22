//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/Ast.h"
#include "Lexer/Lexer.h"

namespace lbc {

class Context;
enum class TokenKind;
AST_FORWARD_DECLARE()

enum class ExprFlags : unsigned {
    CommaAsAnd = 1,
    AssignAsEqual = 2,
    Default = AssignAsEqual
};
ENABLE_BITMASK_OPERATORS(ExprFlags);

class Parser final {
public:
    NO_COPY_AND_MOVE(Parser)

    Parser(Context& context, unsigned int fileId, bool isMain) noexcept;
    ~Parser() noexcept = default;

    [[nodiscard]] unique_ptr<AstModule> parse() noexcept;

private:
    enum class Scope {
        Root,
        Function
    };

    [[nodiscard]] unique_ptr<AstStmtList> stmtList() noexcept;
    [[nodiscard]] unique_ptr<AstStmt> statement() noexcept;
    [[nodiscard]] unique_ptr<AstStmt> declaration() noexcept;

    [[nodiscard]] unique_ptr<AstExpr> expression(ExprFlags flags = ExprFlags::Default) noexcept;
    [[nodiscard]] unique_ptr<AstExpr> factor() noexcept;
    [[nodiscard]] unique_ptr<AstExpr> primary() noexcept;
    [[nodiscard]] unique_ptr<AstExpr> unary(llvm::SMRange range, TokenKind op, unique_ptr<AstExpr> expr) noexcept;
    [[nodiscard]] unique_ptr<AstExpr> binary(llvm::SMRange range,TokenKind op, unique_ptr<AstExpr> lhs, unique_ptr<AstExpr> rhs) noexcept;
    [[nodiscard]] unique_ptr<AstExpr> expression(unique_ptr<AstExpr> lhs, int precedence) noexcept;
    [[nodiscard]] unique_ptr<AstIdentExpr> identifier() noexcept;
    [[nodiscard]] unique_ptr<AstLiteralExpr> literal() noexcept;
    [[nodiscard]] unique_ptr<AstCallExpr> callExpr() noexcept;
    [[nodiscard]] unique_ptr<AstIfExpr> ifExpr() noexcept;
    [[nodiscard]] std::vector<unique_ptr<AstExpr>> expressionList() noexcept;

    [[nodiscard]] unique_ptr<AstAssignStmt> assignment() noexcept;
    [[nodiscard]] unique_ptr<AstExprStmt> callStmt() noexcept;
    [[nodiscard]] unique_ptr<AstVarDecl> kwVar(unique_ptr<AstAttributeList> attribs) noexcept;
    [[nodiscard]] unique_ptr<AstIfStmt> kwIf() noexcept;
    [[nodiscard]] AstIfStmt::Block ifBlock() noexcept;
    [[nodiscard]] AstIfStmt::Block thenBlock(std::vector<unique_ptr<AstVarDecl>> decls, unique_ptr<AstExpr> expr) noexcept;
    [[nodiscard]] unique_ptr<AstForStmt> kwFor() noexcept;

    [[nodiscard]] unique_ptr<AstAttributeList> attributeList() noexcept;
    [[nodiscard]] unique_ptr<AstAttribute> attribute() noexcept;
    [[nodiscard]] std::vector<unique_ptr<AstLiteralExpr>> attributeArgList() noexcept;

    [[nodiscard]] unique_ptr<AstTypeExpr> typeExpr() noexcept;

    [[nodiscard]] unique_ptr<AstFuncDecl> kwDeclare(unique_ptr<AstAttributeList> attribs) noexcept;
    [[nodiscard]] unique_ptr<AstFuncDecl> funcSignature(llvm::SMLoc start, unique_ptr<AstAttributeList> attribs, bool hasImpl) noexcept;
    [[nodiscard]] std::vector<unique_ptr<AstFuncParamDecl>> funcParamList(bool& isVariadic) noexcept;
    [[nodiscard]] unique_ptr<AstFuncParamDecl> funcParam() noexcept;
    [[nodiscard]] unique_ptr<AstFuncStmt> kwFunction(unique_ptr<AstAttributeList> attribs) noexcept;
    [[nodiscard]] unique_ptr<AstStmt> kwReturn() noexcept;

    // return true if has more content to parse
    [[nodiscard]] bool isValid() const noexcept;

    // match current token against kind
    [[nodiscard]] bool match(TokenKind kind) const noexcept;

    // replace token kind with another (e.g. Minus to Negate)
    void replace(TokenKind what, TokenKind with) noexcept;

    // expect token to match, move to next token and return current
    // return nullptr otherwise
    [[nodiscard]] unique_ptr<Token> accept(TokenKind kind) noexcept;

    // expect token to match, move to next token and return current
    // show error and terminate otherwise
    unique_ptr<Token> expect(TokenKind kind) noexcept;

    // advance to the next token from the stream
    unique_ptr<Token> move() noexcept;

    // show error and terminate compilation
    [[noreturn]] void error(const Twine& message) noexcept;

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
