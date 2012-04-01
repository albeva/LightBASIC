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
    m_callStmt(false)
{
}


/**
 * Process the expression. If type coercion is required
 * then insert AstCastExpr node in front of current ast expression
 */
void SemanticAnalyser::expression(unique_ptr<AstExpression> & ast, Type * cast, CastPolicy policy)
{
    // current symbol
    SCOPED_GUARD(m_id);
    SCOPED_GUARD(m_symbol);
    
    // the expression
    ast->accept(this);
    
    // cast
    if (cast) coerce(ast, cast, policy);
}


/**
 * coerce expression to a type if needed. Allow only "safe" implicit casts
 */
void SemanticAnalyser::coerce(unique_ptr<AstExpression> & ast, Type * type, CastPolicy policy)
{
    if (!type->compare(ast->type)) {
        
        auto rightType = ast->type;
        auto leftType = type;
        
        // strict cast policy:
        // - no data narrowing (int -> short)
        // - no casts between incompatible pointer types (except nullptr and any ptr)
        if (policy == CastPolicy::Strict) {
            // check for incompatible pointer type assignments
            if (leftType->isPointer()) {
                if (!rightType->isPointer()) {
                    THROW_EXCEPTION(string("Assigning a non pointer to a pointer: ") + rightType->toString());
                }
                if (!rightType->IsAnyPtr() && !leftType->IsAnyPtr() && !leftType->compare(rightType)) {
                    THROW_EXCEPTION(string("Mismatching pointer types: ") + leftType->toString() + " and " + rightType->toString());
                }
            } else if (rightType->isPointer()) {
                THROW_EXCEPTION(string("Invalid type conversion from ") + leftType->toString() + " to " + rightType->toString());
            }
        }
        
        // create cast expression and inject it into the ast tree
        auto expr = ast.release();
        ast.reset(new AstCastExpr(expr));
        ast->type = type;
    }
}


//
// AstProgram
void SemanticAnalyser::visit(AstProgram * ast)
{
    // reset the state
    m_table = nullptr;
    m_symbol = nullptr;
    m_type = nullptr;
    m_callStmt = false;
    
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
    // process the id
    ast->signature->id->accept(this);
    
    // check if already exists
    if (m_symbol && m_symbol->scope() == m_table) {
        THROW_EXCEPTION(string("Duplicate symbol: ") + m_symbol->id());
    }
    
    // analyse the signature. This will create a type
    ast->signature->accept(this);
    
    // create the symbol
    m_symbol = new Symbol(m_id, m_type, ast, nullptr);
    
    // attributes
    if (ast->attribs) ast->attribs->accept(this);
    
    // store in the ast. Should Ast own the symbol or the symbol table?
    ast->symbol = m_symbol;
    
    // store in the symbol table
    m_table->add(m_symbol);
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
    // if has a type then it is a function
    if (ast->typeExpr) {
        ast->typeExpr->accept(this);
        funcType->result(m_type);
    }
    // var arg?
    funcType->vararg = ast->vararg;
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
    
    // is any ptr?
    if (kind == TokenType::Any) {
        m_type = PtrType::getAnyPtr();
        if (ast->level > 1) m_type = PtrType::get(m_type, ast->level);
    } else {
        // basic type
        m_type = PrimitiveType::get(kind);
        if (!m_type) {
            THROW_EXCEPTION(string("Invalid type: ") + ast->token->name());
        }
        // is it a pointer?
        if (ast->level) m_type = PtrType::get(m_type, ast->level);
    }
}


//
// AstFunctionStmt
void SemanticAnalyser::visit(AstFunctionStmt * ast)
{
    // process the id
    ast->signature->id->accept(this);
    
    // already implemented?
    if (m_symbol && m_symbol->scope() == m_table && m_symbol->impl()) {
        THROW_EXCEPTION(string("Type '") + m_symbol->id() + "' is already implemented");
    }
    
    // process the signature
    ast->signature->accept(this);
    
    // m_type contains the type of the signature. if symbol was found
    // then check that types match.
    if (m_symbol) {
        if (!m_symbol->type()->compare(m_type)) {
            THROW_EXCEPTION(string("Type mismatch between '") + m_symbol->id() + "' and '" + m_symbol->id());
        }
        m_symbol->impl(ast);
        if (ast->attribs) {
            THROW_EXCEPTION(string("Duplicate attribute declarations"));
        }
    } else {
        m_symbol = new Symbol(m_id, m_type, ast, ast);
        ast->symbol = m_symbol;
        m_table->add(m_symbol);
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
            m_table->add(sym);
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
    SCOPED_GUARD(m_type);
    
    // id
    ast->id->accept(this);
    
    // exists?
    if (m_symbol && m_symbol->scope() == m_table) {
        THROW_EXCEPTION(string("Duplicate definition of ") + m_symbol->id());
    }
    
    // declared with DIM
    if (ast->typeExpr) {
        // get the type
        ast->typeExpr->accept(this);
        
        // can this type be instantiated?
        if (!m_type->isInstantiable()) {
            THROW_EXCEPTION(string("Cannot declare a variable with type ") + ast->typeExpr->token->lexeme());
        }
        
        // has an initalizer expression?
        if (ast->expr) {
            expression(ast->expr, m_type);
        }
    }
    // declared with VAR
    else {
        expression(ast->expr);
        m_type = ast->expr->type;
        // can this type be instantiated?
        if (!m_type->isInstantiable()) {
            THROW_EXCEPTION(string("Cannot declare a variable of type ") + m_type->toString());
        }
    }
    
    // create new symbol
    m_symbol = new Symbol(m_id, m_type, ast, ast);
    ast->symbol = m_symbol;
    
    // attributes
    if (ast->attribs) ast->attribs->accept(this);
    
    // add to the symbol table
    m_table->add(m_symbol);
}


//
// AstAssignStmt
void SemanticAnalyser::visit(AstAssignStmt * ast)
{
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
    if (left->is(Ast::IdentExpr)) {
        // process
        left->accept(this);
        
        // check
        if (!m_symbol) {
            THROW_EXCEPTION(string("Use of undeclared '") + m_id + "'");
        } else if (!m_symbol->type()->isInstantiable()) {
            THROW_EXCEPTION(string("Cannot assign to '") + m_symbol->id() + "' of type " + m_symbol->type()->toString());
        }
        leftType = m_symbol->type();
        
        help = string("identifier ") + m_symbol->id();
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
        leftType = pt->dereference(deref);
        if (!leftType->isInstantiable()) {
            THROW_EXCEPTION(string("Cannot assign to '") + help + "' of type " + leftType->toString());
        }
    }
    
    // do type casting
    // this can throw if incompatible types
    ast->right->accept(this);
    coerce(ast->right, leftType);
}


//
// AstCallStmt
void SemanticAnalyser::visit(AstCallStmt * ast)
{
    m_callStmt = true;
    ast->expr->accept(this);
    m_callStmt = false;
}


//
// AstIfStmt
void SemanticAnalyser::visit(AstIfStmt * ast)
{
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
// AstForStmt
void SemanticAnalyser::visit(AstForStmt * ast)
{
    // new scope
    SCOPED_GUARD(m_table);
    m_table = new SymbolTable(m_table);
    
    // expr = initializer
    ast->stmt->accept(this);
    Type * type = nullptr;
    if (ast->stmt->is(Ast::VarDecl)) {
        type = static_cast<AstVarDecl *>(ast->stmt.get())->symbol->type();
    } else if (ast->stmt->is(Ast::AssignStmt)) {
        type = static_cast<AstAssignStmt *>(ast->stmt.get())->left->type;
    }
    
    // TP end condition. cast
    expression(ast->end, type);
    
    // step ?
    if (ast->step) {
        expression(ast->step, type);
    }
    
    // the body
    ast->block->symbolTable = m_table;
    ast->block->accept(this);
}


//
// AstAddressOfExpr
void SemanticAnalyser::visit(AstAddressOfExpr * ast)
{
    ast->id->accept(this);
    if (!m_symbol) {
        THROW_EXCEPTION(string("Use of undeclared identifier ") + m_id);
    }
    ast->type = PtrType::get(ast->id->type, 1);
}


//
// AstDereferenceExpr
void SemanticAnalyser::visit(AstDereferenceExpr * ast)
{
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
    ast->lhs->accept(this);
    auto left = ast->lhs->type;
    
    ast->rhs->accept(this);
    auto right = ast->rhs->type;
    
    if (ast->token->type() == TokenType::Modulus) {
        if (!left->isIntegral() || !right->isIntegral()) {
            THROW_EXCEPTION(string("Invalid operand to binary expression(") + left->toString() + ", " + right->toString() + ")");
        }
        if (left->getSizeInBits() > right->getSizeInBits()) {
            coerce(ast->rhs, left);
            ast->type = left;
        } else if (left->getSizeInBits() < right->getSizeInBits()) {
            coerce(ast->lhs, right);
            ast->type = right;
        } else {
            ast->type = left;
        }
    } else {
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
            else {
                THROW_EXCEPTION(string("Invalid type to binary expression(") + left->toString() + ", " + right->toString() + ")");
            }
        }
        // fp ->
        else if (left->isFloatingPoint()) {
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
            else {
                THROW_EXCEPTION(string("Invalid type to binary expression(") + left->toString() + ", " + right->toString() + ")");
            }
        }
        // ptr
        else if (left->isPointer()) {
            if (right->isPointer()) {
                if (left->IsAnyPtr() && !right->IsAnyPtr()) {
                    coerce(ast->lhs, right);
                } else if (!left->IsAnyPtr() && right->IsAnyPtr()) {
                    coerce(ast->rhs, left);
                } else if (!right->compare(left)) {
                    THROW_EXCEPTION(string("Invalid type to binary expression(") + left->toString() + ", " + right->toString() + ")");
                }
            } else {
                THROW_EXCEPTION(string("Invalid type to binary expression(") + left->toString() + ", " + right->toString() + ")");
            }
        }
        
        // result of logical comparison
        ast->type = PrimitiveType::get(TokenType::Bool);
    }
}


//
// AstCallExpr
void SemanticAnalyser::visit(AstCallExpr * ast)
{
    // the id
    ast->id->accept(this);
    
    // find
    if (!m_symbol) {
        THROW_EXCEPTION(string("Use of undeclared identifier '") + m_id + "'");
    }
    
    // not a callable function?
    if (m_symbol->type()->kind() != Type::Function) {
        THROW_EXCEPTION(string("Called identifier '") + m_symbol->id() + "' is not a function");
    }
    
    // get function type
    auto type = static_cast<FunctionType *>(m_symbol->type());
    
    // check if this is a sub or a function call
    if (m_callStmt) {
        m_callStmt = false;
    } else if (type->result() == nullptr) {
        THROW_EXCEPTION(string("Cannot use SUB ") + m_id + " in an expression");
    }
    
    // check the parameter types against the argument types
    if (ast->args) {
        if (type->vararg && ast->args->args.size() < type->params.size()) {
            THROW_EXCEPTION("Argument count mismatch");
        }
        else if (!type->vararg && type->params.size() != ast->args->args.size()) {
            THROW_EXCEPTION("Argument count mismatch");
        }
        int i = 0;
        for(auto & arg : ast->args->args) {
            auto cast = type->params.size() > i ? type->params[i++] : nullptr;
            expression(arg, cast);
            // cast var args
            if (!cast) {
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
    switch (ast->token->type()) {
        case TokenType::StringLiteral:
            ast->type = PtrType::get(PrimitiveType::get(TokenType::Byte), 1);
            break;
        case TokenType::IntegerLiteral:
            ast->type = PrimitiveType::get(TokenType::Integer);
            break;
        case TokenType::FloatingPointLiteral:
            ast->type = PrimitiveType::get(TokenType::Double);
            break;
        case TokenType::True:
        case TokenType::False:
            ast->type = PrimitiveType::get(TokenType::Bool);
            break;
        case TokenType::Null:
            ast->type = PtrType::getAnyPtr();
            break;            
        default:
            THROW_EXCEPTION("Invalid type");
            break;
    }
}


//
// AstIdentExpr
void SemanticAnalyser::visit(AstIdentExpr * ast)
{
    // the id
    m_id = ast->token->lexeme();
    m_symbol = m_table->get(m_id);
    if (m_symbol) {
        ast->type = m_symbol->type();
    }
}


//
// AstReturnStmt
void SemanticAnalyser::visit(AstReturnStmt * ast)
{
    assert(m_type->isFunction());
    auto funcType = static_cast<FunctionType *>(m_type);
    
    if (funcType->result()) {
        if (!ast->expr) {
            THROW_EXCEPTION("Expected expression");
        }
        expression(ast->expr, funcType->result());
    } else if (ast->expr) {
        THROW_EXCEPTION(string("Unexpected expression"));
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
