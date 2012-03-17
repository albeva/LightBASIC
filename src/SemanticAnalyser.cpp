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
:   RecursiveAstVisitor(true),
    m_table(nullptr),
    m_symbol(nullptr),
    m_type(nullptr),
    m_coerceType(nullptr)
{
}


//
// AstProgram
void SemanticAnalyser::visit(AstProgram * ast)
{
    // reset the state
    m_table = nullptr;
    m_symbol = nullptr;
    m_type = nullptr;
    m_coerceType = nullptr;
    
    // create new scope
    ast->symbolTable = make_shared<SymbolTable>(m_table);
    m_table = ast->symbolTable.get();
    // visit all declarations
    for (auto & decl : ast->decls) decl->accept(this);
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
        THROW_EXCEPTION(string("Duplicate symbol: ") + id);
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
    auto funcType = new FunctionType();
    // process params
    m_type = funcType;
    if (ast->params) ast->params->accept(this);
    // process result type
    if (ast->typeExpr) {
        ast->typeExpr->accept(this);
        funcType->result(m_type);
    }
    // var arg?
    funcType->vararg = ast->vararg;
    //
    m_type = funcType;
}


//
// AstFuncParam
void SemanticAnalyser::visit(AstFuncParam * ast)
{
    auto funcType = static_cast<FunctionType *>(m_type);
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
    // basic type
    m_type = PrimitiveType::get(kind);
    if (!m_type) {
        THROW_EXCEPTION(string("Invalid type: ") + ast->token->name());
    }
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
        THROW_EXCEPTION(string("Type '") + id + "' is already implemented");
    }
    
    // process the signature
    ast->signature->accept(this);
    
    // m_type contains the type of the signature. if symbol was found
    // then check that types match.
    if (m_symbol) {
        if (!m_symbol->type()->compare(m_type)) {
            THROW_EXCEPTION(string("Type mismatch between '") + id + "' and '" + m_symbol->id());
        }
        m_symbol->impl(ast);
        if (ast->attribs) {
            THROW_EXCEPTION(string("Duplicate attribute declarations"));
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
    auto funcType = static_cast<FunctionType *>(m_type);
    
    // fill the table with parameters
    if (ast->signature->params) {
        int i = 0;
        for(auto & param : ast->signature->params->params) {
            const string & paramId = param->id->token->lexeme();
            if (m_table->exists(paramId)) {
                THROW_EXCEPTION(string("Duplicate defintion of ") + paramId);
            }
            auto sym = new Symbol(paramId, funcType->params[i++], param.get(), nullptr);
            m_table->add(paramId, sym);
            param->symbol = sym;
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
        THROW_EXCEPTION(string("Duplicate definition of ") + id);
    }
    
    // get the type
    ast->typeExpr->accept(this);
    
    // can this type be instantiated?
    if (!m_type->isInstantiable()) {
        THROW_EXCEPTION(string("Cannot declare a variable with type ") + ast->typeExpr->token->lexeme());
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
    SCOPED_GUARD(m_coerceType);
    
    // id
    auto left = ast->left.get();
    auto deref = 0;
    while (left->is(Ast::DereferenceExpr)) {
        deref++;
        left = static_cast<AstDereferenceExpr*>(left)->expr.get();
    }
    
    // not identifier
    string help;
    Type * leftType = nullptr;
    if (left->kind() == Ast::IdentExpr) {
        const string & id = static_cast<AstIdentExpr *>(left)->token->lexeme();
        
        // get the symbol
        auto symbol = m_table->get(id);
        
        // check
        if (!symbol) {
            THROW_EXCEPTION(string("Use of undeclared '") + id + "'");
        } else if (!symbol->type()->isInstantiable()) {
            THROW_EXCEPTION(string("Cannot assign to '") + id + "' of type " + symbol->type()->toString());
        }
        leftType = symbol->type();
        
        help = string("identifier ") + symbol->id();
    } else {
        left->accept(this);
        leftType = left->type;
        help = "expression";
    }
    
    // check pointer deref
    if (deref != 0) {
        if (!leftType->isPointer()) {
            THROW_EXCEPTION("Dereferencing a non pointer of type " + leftType->toString());
        }
        auto pt = static_cast<PtrType *>(leftType);
        if (pt->indirection() < deref) {
            THROW_EXCEPTION(string("Dereferencing ") + help + " of type " + pt->toString() + " too many levels");
        }
        m_coerceType = pt->indirection() - deref == 0
                     ? pt->getBaseType()
                     : PtrType::get(pt->getBaseType(), pt->indirection() - deref);
        if (!m_coerceType->isInstantiable()) {
            THROW_EXCEPTION(string("Cannot assign to '") + help + "' of type " + m_coerceType->toString());
        }
    } else {
        m_coerceType = leftType;
    }
    
    // m_type will hold the result of the expression
    expression(ast->right);
}


//
// AstCallStmt
void SemanticAnalyser::visit(AstCallStmt * ast)
{
    ast->expr->accept(this);
}


//
// AstIfStmt
void SemanticAnalyser::visit(AstIfStmt * ast)
{
    m_coerceType = nullptr; //PrimitiveType::get(TokenType::Bool);
    ast->expr->accept(this);
    coerce(ast->expr, PrimitiveType::get(TokenType::Bool));
    
    // true block
    {
        SCOPED_GUARD(m_table);
        m_table = new SymbolTable(m_table);
        static_cast<AstStmtList *>(ast->trueBlock.get())->symbolTable = m_table;
        ast->trueBlock->accept(this);
    }
    
    // false block
    // in false the ast node *might* be an Else If
    // statement. Don't know if this is the most elegant
    // aproach however...
    if (ast->falseBlock) {
        SCOPED_GUARD(m_table);
        if (ast->falseBlock->is(Ast::StmtList)) {
            m_table = new SymbolTable(m_table);
            static_cast<AstStmtList *>(ast->falseBlock.get())->symbolTable = m_table;
        }
        ast->falseBlock->accept(this);
    }
}


//
// Process the expression. If type coercion is required
// then insert AstCastExpr node in front of current ast expression
void SemanticAnalyser::expression(unique_ptr<AstExpression> & ast)
{
    // process the expression
    ast->accept(this);
    
    if (m_coerceType) coerce(ast, m_coerceType);
}


//
// coerce expression to a type if needed
void SemanticAnalyser::coerce(unique_ptr<AstExpression> & ast, Type * type)
{
    // deal with type coercion. If can create implicit cast node
    if (!type->compare(ast->type)) {
        // check if this is signed -> unsigned or vice versa
        if (ast->type->isIntegral() && type->isIntegral()) {
            if (ast->type->getSizeInBits() == type->getSizeInBits()) {
                return;
            }
        }
        // prefix expression with AstCastExpr
        auto expr = ast.release();
        ast.reset(new AstCastExpr(expr));
        ast->type = type;
    }
}


//
// AstAddressOfExpr
void SemanticAnalyser::visit(AstAddressOfExpr * ast)
{
    SCOPED_GUARD(m_coerceType);
    m_coerceType = nullptr;
    ast->id->accept(this);
    ast->type = PtrType::get(ast->id->type, 1);
}


//
// AstDereferenceExpr
void SemanticAnalyser::visit(AstDereferenceExpr * ast)
{
    SCOPED_GUARD(m_coerceType);
    m_coerceType = nullptr;
    ast->expr->accept(this);
    if (!ast->expr->type->isPointer()) {
        THROW_EXCEPTION("Dereferencing a non pointer");
    }
    ast->type = static_cast<PtrType *>(ast->expr->type)->dereference();
}


//
// AstBinaryExpr
void SemanticAnalyser::visit(AstBinaryExpr * ast)
{
    SCOPED_GUARD(m_coerceType);
    m_coerceType = nullptr;
    
    Type * left = nullptr, * right = nullptr;
    if (ast->lhs->isConstant() && !ast->rhs->isConstant()) {
        ast->rhs->accept(this);
        right = ast->rhs->type;
        
        m_coerceType = right;
        ast->lhs->accept(this);
        left = ast->lhs->type;
    } else if (!ast->lhs->isConstant() && ast->rhs->isConstant()) {
        ast->lhs->accept(this);
        left = ast->lhs->type;
        
        m_coerceType = left;
        ast->rhs->accept(this);
        right = ast->rhs->type;        
    } else {
        ast->lhs->accept(this);
        left = ast->lhs->type;
        
        // should be okay to try and coerce the right hand side
        m_coerceType = left;
        
        ast->rhs->accept(this);
        right = ast->rhs->type;
    }
    
    // int -> 
    if (left->isIntegral()) {
        // -> int
        if (right->isIntegral()) {
            if (left->getSizeInBits() > right->getSizeInBits()) {
                coerce(ast->rhs, left);
            } else if (left->getSizeInBits() < right->getSizeInBits()) {
                coerce(ast->lhs, right);
            }
        }
        // -> fp
        else if (right->isFloatingPoint()) {
            coerce(ast->lhs, right);
        }
        // -> ptr
        else if (right->isPointer()) {
            if (ast->lhs->is(Ast::LiteralExpr)) {
                coerce(ast->lhs, right);
            } else {
                THROW_EXCEPTION("Comparison between integer and a pointer");
            }
        }
    }
    // fp ->
    if (left->isFloatingPoint()) {
        // -> int
        if (right->isIntegral()) {
            coerce(ast->rhs, left);
        }
        // -> fp
        else if (right->isFloatingPoint()) {
            if (left->getSizeInBits() > right->getSizeInBits()) {
                coerce(ast->rhs, left);
            } else if (left->getSizeInBits() < right->getSizeInBits()) {
                coerce(ast->lhs, right);
            }
        }
        // ->ptr
        else if (right->isPointer()) {
            THROW_EXCEPTION("Comparison between floating point and a pointer");
        }
    }
    // ptr
    if (left->isPointer()) {
        // -> int
        if (right->isIntegral()) {
            if (ast->rhs->is(Ast::LiteralExpr)) {
                coerce(ast->rhs, left);
            } else {
                THROW_EXCEPTION("Comparison between integer and a pointer");
            }
        }
        // -> fp
        else if (right->isFloatingPoint()) {
            THROW_EXCEPTION("Comparison between floating point and a pointer");
        }
        // -> ptr
        else if (right->isPointer()) {
            if (!left->compare(right)) {
                THROW_EXCEPTION("Comparison between distinct pointer types");
            }
        }
    }
    
    // result of logical comparison
    ast->type = PrimitiveType::get(TokenType::Bool);
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
        THROW_EXCEPTION(string("Use of undeclared identifier '") + id + "'");
    }
    
    // not a callable function?
    if (sym->type()->kind() != Type::Function) {
        THROW_EXCEPTION(string("Called identifier '") + id + "' is not a function");
    }
    
    auto type = static_cast<FunctionType *>(sym->type());
    
    // check the parameter types against the argument types
    if (ast->args) {
        if (type->vararg && ast->args->args.size() < type->params.size()) {
            THROW_EXCEPTION("Argument count mismatch");
        }
        else if (!type->vararg && type->params.size() != ast->args->args.size()) {
            THROW_EXCEPTION("Argument count mismatch");
        }
        int i = 0;
        SCOPED_GUARD(m_coerceType);
        for(auto & arg : ast->args->args) {
            m_coerceType = type->params.size() < i + 1 ? type->params[i++] : nullptr;
            expression(arg);
            // cast var args
            if (!m_coerceType) {
                // extend int to 32 bit at least
                if (arg->type->isIntegral() && arg->type->getSizeInBits() < 32) {
                    coerce(arg, PrimitiveType::get(TokenType::Integer));
                }
                // extend single to double. Is this 64bit only?
                else if (arg->type->isFloatingPoint() && arg->type->getSizeInBits() == 32) {
                    coerce(arg, PrimitiveType::get(TokenType::Double));
                }
            }
        }
    } else if (type->params.size() != 0) {
        THROW_EXCEPTION("Argument count mismatch");
    }
    
    // set the expression type
    ast->type = type->result();
}


//
// AstLiteralExpr
void SemanticAnalyser::visit(AstLiteralExpr * ast)
{
    auto type = ast->token->type();
    if (type == TokenType::StringLiteral) {
        ast->type = PtrType::get(PrimitiveType::get(TokenType::Byte), 1);
    } else if (type == TokenType::IntegerLiteral || type == TokenType::FloatingPointLiteral) {
        if (m_coerceType) {
            ast->type = m_coerceType;
        } else{
            ast->type = PrimitiveType::get(type == TokenType::IntegerLiteral ? TokenType::Integer : TokenType::Double);
        }
    } else if (type == TokenType::True || type == TokenType::False) {
        if (m_coerceType) {
            ast->type = m_coerceType;
        } else {
            ast->type = PrimitiveType::get(TokenType::Bool);
        }   
    } else if (type == TokenType::Any) {
        THROW_EXCEPTION("ANY not implemented");
    } else {
        THROW_EXCEPTION("Invalid type");
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
        THROW_EXCEPTION(string("Use of undeclared identifier '") + id + "'");
    }
    
    // set type
    ast->type = sym->type();
}


//
// AstReturnStmt
void SemanticAnalyser::visit(AstReturnStmt * ast)
{
    assert(m_type->isFunction());
    auto funcType = static_cast<FunctionType *>(m_type);
    
    if (ast->expr) {
        SCOPED_GUARD(m_coerceType);
        m_coerceType = funcType->result();
        expression(ast->expr);
    }
}


//
// AstAttribute
void SemanticAnalyser::visit(AstAttribute * ast)
{
    const string & id = ast->id->token->lexeme();
    if (id == "ALIAS") {
        if (!ast->params) 
            THROW_EXCEPTION("Alias expects a string value");
        if (ast->params->params.size() != 1)
            THROW_EXCEPTION("Alias expects one string value");
        if (ast->params->params[0]->token->type() != TokenType::StringLiteral)
            THROW_EXCEPTION("Incorrect Alias value. String expected");
        if (m_symbol) {
            m_symbol->alias(ast->params->params[0]->token->lexeme());
        }
    }
}













