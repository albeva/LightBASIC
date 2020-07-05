//
// Created by Albert on 03/07/2020.
//
#pragma once
#include "pch.h"
#include "Ast/Ast.def.h"

namespace lbc {

class Lexer;
class Token;
enum class TokenKind;
AST_FORWARD_DECLARE()

class Parser final : noncopyable {
public:
    Parser(llvm::SourceMgr& srcMgr, unsigned int fileId);

    ~Parser();

    unique_ptr<AstProgram> parse();

private:

    unique_ptr<AstStmtList> stmtList();
    unique_ptr<AstStmt> statement();
    unique_ptr<AstExpr> expression();
    unique_ptr<AstIdentExpr> identifier();
    unique_ptr<AstAssignStmt> assignStmt();

    unique_ptr<AstExprStmt> callStmt();
    unique_ptr<AstCallExpr> callExpr();
    vector<unique_ptr<AstExpr>> expressionList();

    unique_ptr<AstVarDecl> kwVar();

    // return true if has more content to parse
    bool isValid() const;

    // match current token against kind
    bool match(TokenKind kind) const;

    // expect token to match, move to next token and return current
    // return nullptr otherwise
    unique_ptr<Token> accept(TokenKind kind);

    // expect token to match, move to next token and return current
    // show error and terminate otherwise
    unique_ptr<Token> expect(TokenKind kind);

    // advance to the next token from the stream
    unique_ptr<Token> move();

    // show error and terminate compilation
    [[ noreturn ]]
    void error(const llvm::Twine& message);

    llvm::SourceMgr& m_srcMgr;
    unsigned m_fileID;
    std::unique_ptr<Lexer> m_lexer;
    std::unique_ptr<Token> m_token;
    std::unique_ptr<Token> m_next;
};

} // namespace lbc