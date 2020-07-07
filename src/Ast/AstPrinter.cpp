//
// Created by Albert on 05/07/2020.
//
#include "AstPrinter.h"
#include "Ast.h"
#include "Lexer/Token.h"
using namespace lbc;

void AstPrinter::visit(const AstProgram *ast) {
    ast->body->accept(this);
}

// Statements

void AstPrinter::visit(const AstStmtList *ast) {
    for (const auto& stmt: ast->stmts) {
        stmt->accept(this);
        std::cout << '\n';
    }
}

void AstPrinter::visit(const AstAssignStmt *ast) {
    ast->ident->accept(this);
    std::cout << " = ";
    ast->expr->accept(this);
}

void AstPrinter::visit(const AstExprStmt *ast) {
    ast->expr->accept(this);
}

// Attributes

void AstPrinter::visit(const AstAttributeList *ast) {
    std::cout << '[';
    bool isFirst = true;
    for (const auto& attr: ast->attribs) {
        if (isFirst) isFirst = false;
        else std::cout << ", ";
        attr->accept(this);
    }
    std::cout << "]";
}

void AstPrinter::visit(const AstAttribute *ast) {
    ast->ident->accept(this);
    if (ast->arguments.size() == 1) {
        std::cout << " = ";
        ast->arguments[0]->accept(this);
    } else if (ast->arguments.size() > 1) {
        bool isFirst = true;
        std::cout << "(";
        for (const auto& arg: ast->arguments) {
            if (isFirst) isFirst = false;
            else std::cout << ", ";
            arg->accept(this);
        }
        std::cout << ")";
    }
}

void AstPrinter::visit(const AstTypeExpr *ast) {
    std::cout << ast->token->lexeme();
}

// Declarations

void AstPrinter::visit(const AstVarDecl *ast) {
    if (ast->attribs) {
        ast->attribs->accept(this);
        std::cout << " _" << '\n';
    }

    std::cout << "VAR ";
    ast->ident->accept(this);

    if (ast->type) {
        std::cout << " AS ";
        ast->type->accept(this);
    }

    if (ast->expr) {
        std::cout << " = ";
        ast->expr->accept(this);
    }
}

void AstPrinter::visit(const AstFuncDecl *ast) {
    if (ast->attribs) {
        ast->attribs->accept(this);
        std::cout << " _" << '\n';
    }

    if (ast->type) {
        std::cout << "FUNCTION ";
    } else {
        std::cout << "SUB ";
    }
    ast->ident->accept(this);

    if (!ast->params.empty()) {
        std::cout << "(";
        bool isFirst = true;
        for (const auto& param: ast->params) {
            if (isFirst) isFirst = false;
            else std::cout << ", ";
            param->accept(this);
        }
        std::cout << ")";
    }

    if (ast->type) {
        std::cout << " AS ";
        ast->type->accept(this);
    }
}

void AstPrinter::visit(const AstFuncParamDecl *ast) {
    ast->ident->accept(this);

    if (ast->type) {
        std::cout << " AS ";
        ast->type->accept(this);
    }

    if (ast->expr) {
        std::cout << " = ";
        ast->expr->accept(this);
    }
}

// Expressions

void AstPrinter::visit(const AstIdentExpr *ast) {
    std::cout << ast->token->lexeme();
}

void AstPrinter::visit(const AstCallExpr *ast) {
    ast->ident->accept(this);
    std::cout << "(";
    bool isFirst = true;
    for (const auto& arg: ast->arguments) {
        if (isFirst) isFirst = false;
        else std::cout << ", ";
        arg->accept(this);
    }
    std::cout << ")";
}

void AstPrinter::visit(const AstLiteralExpr *ast) {
    if (ast->token->kind() == TokenKind::StringLiteral) {
        std::cout << '"' << ast->token->lexeme() << '"';
    } else {
        std::cout << ast->token->lexeme();
    }
}
