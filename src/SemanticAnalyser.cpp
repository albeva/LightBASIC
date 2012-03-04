//
//  SemanticAnalyser.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 26/02/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//

#include "SemanticAnalyser.h"
#include "Ast.h"
#include "Token.h"
#include "Type.h"
#include "Symbol.h"
#include "SymbolTable.h"


#define LOG_VAR(var)    std::cout << #var << " = " << (var) << " : " << __FUNCTION__ << "(" << __LINE__ << ")\n";
#define LOG(msg)        std::cout << msg << " : " << __FUNCTION__ << "(" << __LINE__ << ")\n";

using namespace lbc;

//
// create new one
SemanticAnalyser::SemanticAnalyser()
:    RecursiveAstVisitor(true),
    m_table(nullptr),
    m_symbol(nullptr)
{
}


//
// AstProgram
void SemanticAnalyser::visit(AstProgram * ast)
{
    // create new scope
    ast->symbolTable = make_shared<SymbolTable>(m_table);
    m_table = ast->symbolTable.get();
    // visit all declarations
    for (auto & decl : ast->decls) decl.accept(this);
    // restore the symbol table
    m_table = m_table->parent();
}


//
// AstFunctionDecl
void SemanticAnalyser::visit(AstFunctionDecl * ast)
{
    // function id
    auto const & id = ast->signature->id->token->lexeme();
    
    // check if already exists
    if (m_table->exists(id)) {
        throw Exception(string("Duplicate symbol: ") + id);
    }
    
    // analyse the signature. This will create a type
    ast->signature->accept(this);
    
    // create the symbol
    m_symbol = new Symbol(id, m_type, ast, nullptr);
    
    // attributes
    if (ast->attribs) ast->attribs->accept(this);
    
    // store in the ast. Should Ast own the symbol or the symbol table?
    ast->symbol = m_symbol;
    
    // store in the symbol table
    m_table->add(id, m_symbol);
}


//
// AstFuncSignature
void SemanticAnalyser::visit(AstFuncSignature * ast)
{
    // create new type
    auto funcType = make_shared<FunctionType>(shared_ptr<Type>());
    // process params
    m_type = funcType;
    if (ast->params) ast->params->accept(this);
    // process result type
    if (ast->typeExpr) {
        ast->typeExpr->accept(this);
        funcType->result(m_type);
    }
    m_type = funcType;
}


//
// AstFuncParam
void SemanticAnalyser::visit(AstFuncParam * ast)
{
    auto funcType = static_pointer_cast<FunctionType>(m_type);
    ast->typeExpr->accept(this);
    funcType->params.push_back(m_type);
    m_type = funcType;
}


//
// AstTypeExpr
void SemanticAnalyser::visit(AstTypeExpr * ast)
{
    // get the kind
    auto kind = ast->token->type();
    if (kind != TokenType::Integer && kind != TokenType::Byte) {
        throw Exception(string("Invalid type: ") + ast->token->name());
    }
    // basic type
    m_type = BasicType::get(kind);
    // is it a pointer?
    if (ast->level) m_type = PtrType::get(m_type, ast->level);
}


//
// AstFunctionStmt
void SemanticAnalyser::visit(AstFunctionStmt * ast)
{
    // function id
    const string & id = ast->signature->id->token->lexeme();
    
    // look up if declared
    m_symbol= m_table->get(id, true);
    
    // already implemented?
    if (m_symbol && m_symbol->impl()) {
        throw Exception(string("Type '") + id + "' is already implemented");
    }
    
    // process the signature
    ast->signature->accept(this);
    
    // m_type contains the type of the signature. if symbol was found
    // then check that types match.
    if (m_symbol) {
        if (!m_symbol->type()->compare(m_type)) {
            throw Exception(string("Type mismatch between '") + id + "' and '" + m_symbol->id());
        }
        m_symbol->impl(ast);
        if (ast->attribs) {
            throw Exception(string("Duplicate attribute declarations"));
        }
    } else {
        m_symbol = new Symbol(id, m_type, ast, ast);
        ast->symbol = m_symbol;
        m_table->add(id, m_symbol);
    }
    
    // attributes
    if (ast->attribs) ast->attribs->accept(this);
    
    // create new symbol table
    m_table = new SymbolTable(m_table);
    ast->stmts->symbolTable = m_table;
    
    // function type
    auto funcType = static_pointer_cast<FunctionType>(m_type);
    
    // fill the table with parameters
    if (ast->signature->params) {
        int i = 0;
        for(auto & param : ast->signature->params->params) {
            const string & paramId = param.id->token->lexeme();
            if (m_table->exists(paramId)) {
                throw Exception(string("Duplicate defintion of ") + paramId);
            }
            auto sym = new Symbol(paramId, funcType->params[i++], &param, nullptr);
            m_table->add(paramId, sym);
            param.symbol = sym;
        }
    }
    
    // process function body
    ast->stmts->accept(this);
    
    // restore symbol table
    m_table = m_table->parent();
}


//
// Variable declaration
void SemanticAnalyser::visit(AstVarDecl * ast)
{
    // backup type
    auto tmp = m_type;
    
    // id
    const string & id = ast->id->token->lexeme();
    
    // exists?
    if (m_table->exists(id)) {
        throw Exception(string("Duplicate definition of ") + id);
    }
    
    // get the type
    ast->typeExpr->accept(this);
    
    // can this type be instantiated?
    if (!m_type->isInstantiable()) {
        throw Exception(string("Cannot declare a variable with type ") + ast->typeExpr->token->lexeme());
    }
    
    // create new symbol
    m_symbol= new Symbol(id, m_type, ast, ast);
    ast->symbol = m_symbol;
    
    // attributes
    if (ast->attribs) ast->attribs->accept(this);
    
    // add to the symbol table
    m_table->add(id, m_symbol);
    
    // restore type
    m_type = tmp;
}


//
// AstAssignStmt
void SemanticAnalyser::visit(AstAssignStmt * ast)
{
    // id
    const string & id = ast->id->token->lexeme();
    
    // get the symbol
    auto symbol = m_table->get(id);
    
    // check
    if (!symbol) {
        throw Exception(string("Use of undeclared identifier '") + id + "'");
    } else if (!symbol->type()->isInstantiable()) {
        throw Exception(string("Cannot assign to identifier '") + id + "'");
    }
    
    // m_type will hold the result of the expression
    ast->expr->accept(this);
    auto exprType = ast->expr->type;
    if (!exprType->compare(symbol->type())) {
        throw Exception(string("Incompatible types"));
    }
}


//
// AstCallStmt
void SemanticAnalyser::visit(AstCallStmt * ast)
{
    ast->expr->accept(this);
}


//
// AstCallExpr
void SemanticAnalyser::visit(AstCallExpr * ast)
{
    // the id
    const string & id = ast->id->token->lexeme();
    
    // find
    auto sym = m_table->get(id);
    if (!sym) {
        throw Exception(string("Use of undeclared identifier '") + id + "'");
    }
    
    // not a callable function?
    if (sym->type()->kind() != Type::Function) {
        throw Exception(string("Called identifier '") + id + "' is not a function");
    }
    
    auto type = static_pointer_cast<FunctionType>(sym->type());
    
    // check the parameter types against the argument types
    if (ast->args) {
        if (type->params.size() != ast->args->args.size()) {
            throw Exception("Argument count mismatch");
        }
        int i = 0;
        for(auto & arg : ast->args->args) {
            arg.accept(this);
            if (!arg.type) {
                throw Exception("Somethign is wrong");
            } else if (!arg.type->compare(type->params[i++])) {
                throw Exception(string("Argument type mismatch"));
            }
        }
    } else if (type->params.size() != 0) {
        throw Exception("Argument count mismatch");
    }
    
    // set the expression type
    ast->type = type->result();
}


//
// AstLiteralExpr
void SemanticAnalyser::visit(AstLiteralExpr * ast)
{
    if (ast->token->type() == TokenType::StringLiteral) {
        ast->type = PtrType::get(BasicType::get(TokenType::Byte), 1);
    } else if (ast->token->type() == TokenType::Integer) {
        ast->type = BasicType::get(TokenType::Integer);
    } else {
        throw Exception("Invalid type");
    }
}


//
// AstIdentExpr
void SemanticAnalyser::visit(AstIdentExpr * ast)
{
    // the id
    const string & id = ast->token->lexeme();
    
    // get from the table
    auto sym = m_table->get(id);
    
    // not found?
    if (!sym) {
        throw Exception(string("Use of undeclared identifier '") + id + "'");
    }
    
    // set type
    ast->type = sym->type();
}


//
// AstReturnStmt
void SemanticAnalyser::visit(AstReturnStmt * ast)
{
    assert(m_type->kind() == Type::Function);
    auto funcType = static_pointer_cast<FunctionType>(m_type);
    
    if (ast->expr) {
        ast->expr->accept(this);
        if (!funcType->result()->compare(ast->expr->type)) {
            throw Exception("Mismatching return type");
        }
    }
}


//
// AstAttribute
void SemanticAnalyser::visit(AstAttribute * ast)
{
    
}













