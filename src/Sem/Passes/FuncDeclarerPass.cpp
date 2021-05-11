//
// Created by Albert Varaksin on 01/05/2021.
//
#include "FuncDeclarerPass.h"
#include "Symbol/SymbolTable.h"
#include "Type/Type.h"
using namespace lbc;
using namespace Sem;

void FuncDeclarerPass::visit(AstModule* ast) noexcept {
    m_table = ast->symbolTable.get();
    for (const auto& stmt : ast->stmtList->stmts) {
        switch (stmt->kind()) {
        case AstKind::FuncDecl:
            visitFuncDecl(static_cast<AstFuncDecl*>(stmt.get()), true); // NOLINT
            break;
        case AstKind::FuncStmt:
            visitFuncDecl(static_cast<AstFuncStmt*>(stmt.get())->decl.get(), false); // NOLINT
            break;
        default:
            break;
        }
    }
}

void FuncDeclarerPass::visitFuncDecl(AstFuncDecl* ast, bool external) noexcept {
    const auto& name = ast->id;
    if (m_table->exists(name)) {
        fatalError("Redefinition of "_t + name);
    }
    auto* symbol = m_table->insert(name);

    // alias?
    if (ast->attributes) {
        if (auto alias = ast->attributes->getStringLiteral("ALIAS")) {
            symbol->setAlias(*alias);
        }
    }

    if (symbol->name() == "MAIN" && symbol->alias().empty()) {
        symbol->setAlias("main");
        symbol->setExternal(true);
    } else {
        symbol->setExternal(external);
    }

    // parameters
    std::vector<const TypeRoot*> paramTypes;
    paramTypes.reserve(ast->paramDecls.size());
    {
        RESTORE_ON_EXIT(m_table);
        ast->symbolTable = make_unique<SymbolTable>(m_table);
        m_table = ast->symbolTable.get();
        for (auto& param : ast->paramDecls) {
            visitFuncParamDecl(param.get());
            paramTypes.emplace_back(param->symbol->type());
        }
    }

    // return typeExpr. subs don't have one so default to Void
    const TypeRoot* retType = nullptr;
    if (ast->retTypeExpr) {
        visitTypeExpr(ast->retTypeExpr.get());
        retType = ast->retTypeExpr->type;
    } else {
        retType = TypeVoid::get();
    }

    // create function symbol
    const auto* type = TypeFunction::get(retType, std::move(paramTypes), ast->variadic);
    symbol->setType(type);
    ast->symbol = symbol;
}

void FuncDeclarerPass::visitFuncParamDecl(AstFuncParamDecl* ast) noexcept {
    auto* symbol = createParamSymbol(ast);

    visitTypeExpr(ast->typeExpr.get());
    symbol->setType(ast->typeExpr->type);

    ast->symbol = symbol;
}

void FuncDeclarerPass::visitTypeExpr(AstTypeExpr* ast) noexcept {
    ast->type = TypeRoot::fromTokenKind(ast->tokenKind);
}

Symbol* FuncDeclarerPass::createParamSymbol(AstFuncParamDecl* ast) noexcept {
    const auto& name = ast->id;
    if (m_table->find(name, false) != nullptr) {
        fatalError("Redefinition of "_t + name);
    }
    auto* symbol = m_table->insert(name);

    // alias?
    if (ast->attributes) {
        if (auto alias = ast->attributes->getStringLiteral("ALIAS")) {
            symbol->setAlias(*alias);
        }
    }

    return symbol;
}