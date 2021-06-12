//
// Created by Albert Varaksin on 01/05/2021.
//
#include "FuncDeclarerPass.hpp"
#include "Ast/Ast.hpp"
#include "Driver/Context.hpp"
#include "Symbol/SymbolTable.hpp"
#include "Type/Type.hpp"
#include "TypePass.hpp"

using namespace lbc;
using namespace Sem;

void FuncDeclarerPass::visit(AstModule& ast) {
    m_table = ast.symbolTable;
    visit(*ast.stmtList);
}

void FuncDeclarerPass::visit(AstStmtList& ast) {
    for (const auto& stmt : ast.stmts) {
        switch (stmt->kind) {
        case AstKind::FuncDecl:
            visitFuncDecl(static_cast<AstFuncDecl&>(*stmt), true);
            break;
        case AstKind::FuncStmt:
            visitFuncDecl(*static_cast<AstFuncStmt&>(*stmt).decl, false);
            break;
        case AstKind::Import: {
            auto& import = static_cast<AstImport&>(*stmt);
            if (import.module) {
                visit(*import.module->stmtList);
            }
            break;
        }
        default:
            break;
        }
    }
}

void FuncDeclarerPass::visitFuncDecl(AstFuncDecl& ast, bool external) {
    const auto& name = ast.name;
    if (m_table->exists(name)) {
        fatalError("Redefinition of "_t + name);
    }
    auto* symbol = m_table->insert(m_context, name);
    auto flags = symbol->getFlags();
    flags.callable = true;
    flags.addressable = true;
    symbol->setFlags(flags);

    // alias?
    if (ast.attributes) {
        if (auto alias = ast.attributes->getStringLiteral("ALIAS")) {
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
    paramTypes.reserve(ast.paramDecls.size());
    {
        RESTORE_ON_EXIT(m_table);
        ast.symbolTable = m_context.create<SymbolTable>(m_table);
        m_table = ast.symbolTable;
        for (auto& param : ast.paramDecls) {
            visitFuncParamDecl(*param);
            paramTypes.emplace_back(param->symbol->type());
        }
    }

    // return typeExpr. subs don't have one so default to Void
    const TypeRoot* retType = nullptr;
    if (ast.retTypeExpr) {
        m_typePass.visit(*ast.retTypeExpr);
        retType = ast.retTypeExpr->type;
    } else {
        retType = TypeVoid::get();
    }

    // create function symbol
    const auto* type = TypeFunction::get(retType, std::move(paramTypes), ast.variadic);
    symbol->setType(type);
    ast.symbol = symbol;
}

void FuncDeclarerPass::visitFuncParamDecl(AstFuncParamDecl& ast) {
    auto* symbol = createParamSymbol(ast);

    m_typePass.visit(*ast.typeExpr);
    symbol->setType(ast.typeExpr->type);

    ast.symbol = symbol;
}

Symbol* FuncDeclarerPass::createParamSymbol(AstFuncParamDecl& ast) {
    const auto& name = ast.name;
    if (m_table->find(name, false) != nullptr) {
        fatalError("Redefinition of "_t + name);
    }
    auto* symbol = m_table->insert(m_context, name);

    // alias?
    if (ast.attributes) {
        if (auto alias = ast.attributes->getStringLiteral("ALIAS")) {
            symbol->setAlias(*alias);
        }
    }

    return symbol;
}