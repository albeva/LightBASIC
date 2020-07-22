////
//// Created by Albert on 08/07/2020.
////
//#include "SemanticAnalyzer.h"
//#include "Ast/Ast.h"
//#include "Lexer/Token.h"
//#include "Symbol/Symbol.h"
//#include "Symbol/SymbolTable.h"
//#include "TypeFirst/TypeFirst.h"
//using namespace lbc;
//
//[[noreturn]] static void error(const string& message) {
//    std::cerr << message << '\n';
//    std::exit(EXIT_FAILURE);
//}
//
//SemanticAnalyzer::SemanticAnalyzer(llvm::LLVMContext& context, llvm::SourceMgr& srcMgr, unsigned fileId)
//: m_context{ context },
//  m_srcMgr{ srcMgr },
//  m_fileId{ fileId } {}
//
//std::any SemanticAnalyzer::visit(AstProgram* ast) {
//    ast->symbolTable = make_unique<SymbolTable>(nullptr);
//
//    m_rootTable = m_table = ast->symbolTable.get();
//    ast->stmtList->accept(this);
//
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstStmtList* ast) {
//    for (auto& stmt : ast->stmts) {
//        stmt->accept(this);
//    }
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstAssignStmt* ast) {
//    ast->identExpr->accept(this);
//    ast->expr->accept(this);
//
//    if (ast->identExpr->type != ast->expr->type) {
//        error("Tye mismatch in assignment");
//    }
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstExprStmt* ast) {
//    ast->expr->accept(this);
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstVarDecl* ast) {
//    auto* symbol = createNewSymbol(ast->token.get());
//
//    // type expr?
//    const TypeRoot* type = nullptr;
//    if (ast->typeExpr) {
//        ast->typeExpr->accept(this);
//        type = ast->typeExpr->type;
//    }
//
//    // expression?
//    if (ast->expr) {
//        ast->expr->accept(this);
//
//        if (type != nullptr) {
//            if (type != ast->expr->type) {
//                error("TypeFirst mismatch");
//            }
//        } else {
//            type = ast->expr->type;
//        }
//    }
//
//    // create function symbol
//    symbol->setType(type);
//    ast->symbol = symbol;
//
//    // alias?
//    if (ast->attributes) {
//        if (const auto* token = ast->attributes->getStringLiteral("ALIAS")) {
//            symbol->setAlias(token->lexeme());
//        }
//    }
//
//    return {};
//}
//
///**
// * Analyze function declaration
// */
//std::any SemanticAnalyzer::visit(AstFuncDecl* ast) {
//    auto* symbol = createNewSymbol(ast->token.get(), m_rootTable);
//
//    // parameters
//    std::vector<const TypeRoot*> paramTypes;
//    paramTypes.reserve(ast->paramDecls.size());
//    {
//        RESTORE_ON_EXIT(m_table)
//        ast->symbolTable = make_unique<SymbolTable>(nullptr);
//        m_table = ast->symbolTable.get();
//        for (auto& param : ast->paramDecls) {
//            param->accept(this);
//            paramTypes.emplace_back(param->symbol->type());
//        }
//    }
//
//    // return typeExpr. subs don't have one so default to Void
//    const TypeRoot* retType = nullptr;
//    if (ast->retTypeExpr) {
//        ast->retTypeExpr->accept(this);
//        retType = ast->retTypeExpr->type;
//    } else {
//        retType = TypeVoid::get();
//    }
//
//    // create function symbol
//    const auto* type = TypeFunction::get(retType, std::move(paramTypes), ast->variadic);
//    symbol->setType(type);
//    ast->symbol = symbol;
//
//    // alias?
//    if (ast->attributes) {
//        if (const auto* token = ast->attributes->getStringLiteral("ALIAS")) {
//            symbol->setAlias(token->lexeme());
//        }
//    }
//
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstFuncParamDecl* ast) {
//    auto* symbol = createNewSymbol(ast->token.get());
//
//    ast->typeExpr->accept(this);
//    symbol->setType(ast->typeExpr->type);
//
//    ast->symbol = symbol;
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstAttributeList* ast) {
//    for (auto& attrib : ast->attribs) {
//        attrib->accept(this);
//    }
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstAttribute* ast) {
//    for (auto& arg : ast->argExprs) {
//        arg->accept(this);
//    }
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstTypeExpr* ast) {
//    ast->type = TypeRoot::fromTokenKind(ast->token->kind());
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstIdentExpr* ast) {
//    const auto& name = ast->token->lexeme();
//    auto* symbol = m_table->find(name, true);
//
//    if (symbol == nullptr) {
//        error("Unknown identifier "s + string(name));
//    }
//
//    if (symbol->type() == nullptr) {
//        error("Identifier "s + string(name) + " has unresolved type");
//    }
//
//    ast->symbol = symbol;
//    ast->type = symbol->type();
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstCallExpr* ast) {
//    ast->identExpr->accept(this);
//    auto* symbol = ast->identExpr->symbol;
//
//    const auto* type = dyn_cast<TypeFunction>(symbol->type());
//    if (type == nullptr) {
//        error("Identifier "s + string(symbol->name()) + " is not a callable type"s);
//    }
//
//    const auto& paramTypes = type->paramTypes();
//    auto& args = ast->argExprs;
//
//    if (type->variadic()) {
//        if (paramTypes.size() > args.size()) {
//            error("Argument count mismatch");
//        }
//    } else if (paramTypes.size() != args.size()) {
//        error("Argument count mismatch");
//    }
//
//    for (size_t index = 0; index < args.size(); index++) {
//        args[index]->accept(this);
//        if (index < paramTypes.size()) {
//            if (paramTypes[index] != args[index]->type) {
//                error("TypeFirst mismatch");
//            }
//        }
//    }
//
//    ast->type = type->retType();
//    return {};
//}
//
//std::any SemanticAnalyzer::visit(AstLiteralExpr* ast) {
//    switch (ast->token->kind()) {
//    case TokenKind::StringLiteral:
//        ast->type = TypeZString::get();
//        break;
//    case TokenKind::BooleanLiteral:
//        ast->type = TypeBool::get();
//        break;
//    case TokenKind::NullLiteral:
//        ast->type = TypePointer::get(TypeAny::get());
//        break;
//    case TokenKind::NumberLiteral:
//        ast->type = TypeRoot::fromTokenKind(TokenKind::Integer);
//        break;
//    default:
//        error("Unsupported literal type");
//    }
//    return {};
//}
//
//Symbol* SemanticAnalyzer::createNewSymbol(Token* token, SymbolTable* table) {
//    if (table == nullptr) {
//        table = m_table;
//    }
//
//    auto* symbol = table->find(token->lexeme(), false);
//    if (symbol != nullptr) {
//        error("Redefinition of " + string(token->lexeme()));
//    }
//
//    symbol = table->insert(make_unique<Symbol>(token->lexeme()));
//    return symbol;
//}
