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

class Parser final : private NonCopyable {
public:
    Parser(Context& context, unsigned int fileId, bool isMain);
    unique_ptr<AstProgram> parse();

private:
    enum class Scope {
        Root,
        Function
    };

    unique_ptr<AstStmtList> stmtList();
    unique_ptr<AstStmt> statement();
    unique_ptr<AstExpr> expression();
    unique_ptr<AstIdentExpr> identifier();
    unique_ptr<AstAssignStmt> assignStmt();

    unique_ptr<AstExprStmt> callStmt();
    unique_ptr<AstCallExpr> callExpr();
    std::vector<unique_ptr<AstExpr>> expressionList();

    unique_ptr<AstDecl> declaration();
    unique_ptr<AstAttributeList> attributeList();
    unique_ptr<AstAttribute> attribute();
    unique_ptr<AstLiteralExpr> literal();
    std::vector<unique_ptr<AstLiteralExpr>> attributeArgumentList();

    unique_ptr<AstVarDecl> kwVar();
    unique_ptr<AstDecl> kwDeclare();
    std::vector<unique_ptr<AstFuncParamDecl>> funcParams(bool& isVariadic);
    unique_ptr<AstTypeExpr> typeExpr();

    // return true if has more content to parse
    [[nodiscard]] bool isValid() const;

    // match current token against kind
    [[nodiscard]] bool match(TokenKind kind) const;

    // expect token to match, move to next token and return current
    // return nullptr otherwise
    unique_ptr<Token> accept(TokenKind kind);

    // expect token to match, move to next token and return current
    // show error and terminate otherwise
    unique_ptr<Token> expect(TokenKind kind);

    // advance to the next token from the stream
    unique_ptr<Token> move();

    // show error and terminate compilation
    [[noreturn]] void error(const llvm::Twine& message);

    Context& m_context;
    unsigned m_fileID;
    bool m_isMain;
    Scope m_scope;
    unique_ptr<Lexer> m_lexer;
    unique_ptr<Token> m_token;
    unique_ptr<Token> m_next;
};

} // namespace lbc
