//
// Created by Albert on 05/07/2020.
//
#include "CodePrinter.h"
#include "Ast.h"
#include "Lexer/Token.h"
using namespace lbc;

std::any CodePrinter::visit(AstProgram* ast) {
    ast->stmtList->accept(this);
    return {};
}

// Statements

std::any CodePrinter::visit(AstStmtList* ast) {
    for (const auto& stmt : ast->stmts) {
        stmt->accept(this);
        std::cout << '\n';
    }
    return {};
}

std::any CodePrinter::visit(AstAssignStmt* ast) {
    ast->identExpr->accept(this);
    std::cout << " = ";
    ast->expr->accept(this);
    return {};
}

std::any CodePrinter::visit(AstExprStmt* ast) {
    ast->expr->accept(this);
    return {};
}

// Attributes

std::any CodePrinter::visit(AstAttributeList* ast) {
    std::cout << '[';
    bool isFirst = true;
    for (const auto& attr : ast->attribs) {
        if (isFirst) {
            isFirst = false;
        } else {
            std::cout << ", ";
        }
        attr->accept(this);
    }
    std::cout << "]";
    return {};
}

std::any CodePrinter::visit(AstAttribute* ast) {
    ast->identExpr->accept(this);
    if (ast->argExprs.size() == 1) {
        std::cout << " = ";
        ast->argExprs[0]->accept(this);
    } else if (ast->argExprs.size() > 1) {
        bool isFirst = true;
        std::cout << "(";
        for (const auto& arg : ast->argExprs) {
            if (isFirst) {
                isFirst = false;
            } else {
                std::cout << ", ";
            }
            arg->accept(this);
        }
        std::cout << ")";
    }
    return {};
}

std::any CodePrinter::visit(AstTypeExpr* ast) {
    std::cout << ast->token->lexeme();
    return {};
}

// Declarations

std::any CodePrinter::visit(AstVarDecl* ast) {
    if (ast->attribs) {
        ast->attribs->accept(this);
        std::cout << " _" << '\n';
    }

    std::cout << "VAR ";
    std::cout << ast->token->lexeme();

    if (ast->typeExpr) {
        std::cout << " AS ";
        ast->typeExpr->accept(this);
    }

    if (ast->expr) {
        std::cout << " = ";
        ast->expr->accept(this);
    }
    return {};
}

std::any CodePrinter::visit(AstFuncDecl* ast) {
    if (ast->attribs) {
        ast->attribs->accept(this);
        std::cout << " _" << '\n';
    }

    if (ast->retTypeExpr) {
        std::cout << "FUNCTION ";
    } else {
        std::cout << "SUB ";
    }
    std::cout << ast->token->lexeme();

    if (!ast->paramDecls.empty()) {
        std::cout << "(";
        bool isFirst = true;
        for (const auto& param : ast->paramDecls) {
            if (isFirst) {
                isFirst = false;
            } else {
                std::cout << ", ";
            }
            param->accept(this);
        }
        std::cout << ")";
    }

    if (ast->retTypeExpr) {
        std::cout << " AS ";
        ast->retTypeExpr->accept(this);
    }

    return {};
}

std::any CodePrinter::visit(AstFuncParamDecl* ast) {
    std::cout << ast->token->lexeme();
    std::cout << " AS ";
    ast->typeExpr->accept(this);
    return {};
}

// Expressions

std::any CodePrinter::visit(AstIdentExpr* ast) {
    std::cout << ast->token->lexeme();
    return {};
}

std::any CodePrinter::visit(AstCallExpr* ast) {
    ast->identExpr->accept(this);
    std::cout << "(";
    bool isFirst = true;
    for (const auto& arg : ast->argExprs) {
        if (isFirst) {
            isFirst = false;
        } else {
            std::cout << ", ";
        }

        arg->accept(this);
    }
    std::cout << ")";
    return {};
}

std::any CodePrinter::visit(AstLiteralExpr* ast) {
    if (ast->token->kind() == TokenKind::StringLiteral) {
        std::cout << '"' << ast->token->lexeme() << '"';
    } else {
        std::cout << ast->token->lexeme();
    }
    return {};
}
