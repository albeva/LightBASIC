//
// Created by Albert Varaksin on 03/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/Ast.def.h"
#include "Lexer/Lexer.h"

namespace lbc {

class Context;
enum class TokenKind;
AST_FORWARD_DECLARE()

class Parser final {
public:
    NO_COPY_AND_MOVE(Parser)

    Parser(Context& context, unsigned int fileId, bool isMain) noexcept;
    ~Parser() = default;

    [[nodiscard]] unique_ptr<AstModule> parse() noexcept;

private:
    enum class Scope {
        Root,
        Function
    };

    [[nodiscard]] unique_ptr<AstStmtList> stmtList() noexcept;
    [[nodiscard]] unique_ptr<AstStmt> statement() noexcept;
    [[nodiscard]] unique_ptr<AstExpr> expression() noexcept;
    [[nodiscard]] unique_ptr<AstIdentExpr> identifier() noexcept;
    [[nodiscard]] unique_ptr<AstAssignStmt> assignStmt() noexcept;
    [[nodiscard]] unique_ptr<AstStmt> kwReturn() noexcept;

    [[nodiscard]] unique_ptr<AstExprStmt> callStmt() noexcept;
    [[nodiscard]] unique_ptr<AstCallExpr> callExpr() noexcept;
    [[nodiscard]] std::vector<unique_ptr<AstExpr>> expressionList() noexcept;

    [[nodiscard]] unique_ptr<AstAttributeList> attributeList() noexcept;
    [[nodiscard]] unique_ptr<AstAttribute> attribute() noexcept;
    [[nodiscard]] unique_ptr<AstLiteralExpr> literal() noexcept;
    [[nodiscard]] std::vector<unique_ptr<AstLiteralExpr>> attributeArgumentList() noexcept;

    [[nodiscard]] unique_ptr<AstVarDecl> kwVar(unique_ptr<AstAttributeList> attribs) noexcept;
    [[nodiscard]] unique_ptr<AstFuncDecl> kwDeclare(unique_ptr<AstAttributeList> attribs) noexcept;
    [[nodiscard]] unique_ptr<AstFuncDecl> funcSignature(llvm::SMLoc start, unique_ptr<AstAttributeList> attribs) noexcept;
    [[nodiscard]] std::vector<unique_ptr<AstFuncParamDecl>> funcParams(bool& isVariadic) noexcept;
    [[nodiscard]] unique_ptr<AstFuncStmt> kwFunction(unique_ptr<AstAttributeList> attribs) noexcept;

    [[nodiscard]] unique_ptr<AstTypeExpr> typeExpr() noexcept;

    // return true if has more content to parse
    [[nodiscard]] bool isValid() const noexcept;

    // match current token against kind
    [[nodiscard]] bool match(TokenKind kind) const noexcept;

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
    llvm::SMLoc m_endLoc;
};

} // namespace lbc
