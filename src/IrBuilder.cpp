//
//  IrBuilder.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 03/03/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "IrBuilder.h"
#include "Token.h"
#include "Ast.h"
#include "Type.h"
#include "Symbol.h"
#include "SymbolTable.h"


using namespace lbc;

//
// create
IrBuilder::IrBuilder()
:   m_module(nullptr),
    m_table(nullptr),
    m_function(nullptr),
    m_block(nullptr),
    m_endIfBlock(nullptr),
    m_value(nullptr),
    m_isElseIf(false)
{
}


//
// AstDeclList
void IrBuilder::visit(AstProgram * ast)
{
    // reset the sate
    m_module = nullptr;
    m_function = nullptr;
    m_block = nullptr;
    m_endIfBlock = nullptr;
    m_value = nullptr;
    m_table = nullptr;
    m_isElseIf = false;
    
    // the module
    m_module = new llvm::Module(ast->name, llvm::getGlobalContext());
    
    // symbol table
    m_table = ast->symbolTable.get();
    
    // process declarations
    for (auto & decl : ast->decls) decl->accept(this);
    
    // verify module integrity
    if (llvm::verifyModule(*m_module, llvm::PrintMessageAction)) {
        // there were errors
        delete m_module;
        m_module = nullptr;
        THROW_EXCEPTION("Failed to build the module");
    }
}


//
// AstStmtList
void IrBuilder::visit(AstStmtList * ast)
{
    SCOPED_GUARD(m_table);
    m_table = ast->symbolTable;
    for (auto & stmt : ast->stmts) stmt->accept(this);
}


//
// AstFuncSignature
void IrBuilder::visit(AstFuncSignature * ast)
{
    // the id
    const string & id = ast->id->token->lexeme();
    Symbol * sym = m_table->get(id);
    
    // get the function
    if (!sym->value) {
        assert(!m_module->getFunction(id));
        // get the symbol
        auto llvmType = sym->type()->llvm();
        string alias = sym->alias();
        if (id == "MAIN") alias = "main";
        // create llvm function
        m_function = llvm::Function::Create(
             llvm::cast<llvm::FunctionType>(llvmType),
             llvm::GlobalValue::ExternalLinkage,
             alias,
             m_module
        );
        m_function->setCallingConv(llvm::CallingConv::C);
        sym->value = m_function;
    }
    
    // bind function params
    if (ast->params) {
        auto llp = m_function->arg_begin();
        for (auto & p : ast->params->params) {
            llp->setName(p->id->token->lexeme());
            if (p->symbol) {
                p->symbol->value = llp;
            }
            llp++;
        }
    }
}


//
// AstFunctionDecl
void IrBuilder::visit(AstFunctionDecl * ast)
{    
    // process the signature
    ast->signature->accept(this);
}


//
// AstFunctionStmt
void IrBuilder::visit(AstFunctionStmt * ast)
{
    // function signature
    ast->signature->accept(this);
        
    // create the block
    SCOPED_GUARD(m_block);
    m_block = llvm::BasicBlock::Create(m_module->getContext(), "", m_function, 0);
    
    // store function pointers locally
    if (ast->signature->params) {
        for (auto & p : ast->signature->params->params) {
            auto sym = p->symbol;
            auto value = sym->value;
            sym->value = new llvm::AllocaInst(sym->type()->llvm(), "", m_block);
            new llvm::StoreInst(value, sym->value, m_block);
        }
    }
    
    // process the body
    ast->stmts->accept(this);
}


//
// AstReturnStmt
void IrBuilder::visit(AstReturnStmt * ast)
{
    m_value = nullptr;
    if (ast->expr) ast->expr->accept(this);
    llvm::ReturnInst::Create(m_module->getContext(), m_value, m_block);
}


//
// AstLiteralExpr
void IrBuilder::visit(AstLiteralExpr * ast)
{
    const string & lexeme = ast->token->lexeme();
    auto & context = m_module->getContext();
    const auto & token = ast->token;
    
    // string literal
    if (token->type() == TokenType::StringLiteral) {
        auto iter = m_stringLiterals.find(lexeme);
        if (iter != m_stringLiterals.end()) {
            m_value = iter->second;
        } else {
            auto arrType = llvm::ArrayType::get(llvm::IntegerType::get(context, 8), lexeme.length() + 1);
            auto global = new llvm::GlobalVariable(
                *m_module,
                arrType,
                true,
                llvm::GlobalValue::PrivateLinkage,
                llvm::ConstantArray::get(context, lexeme, true),
                ".str"
            );
            global->setAlignment(1);
            
            std::vector<llvm::Constant*> indeces;
            llvm::ConstantInt * const_int64 = llvm::ConstantInt::get(context, llvm::APInt(32, 0, false));
            indeces.push_back(const_int64);
            indeces.push_back(const_int64);
            m_value = llvm::ConstantExpr::getGetElementPtr(global, indeces);
            m_stringLiterals[lexeme] = m_value;
        }
        return;
    }
    
    auto local = ast->type;
    auto type = local->llvm();
    if (local->isBoolean() || token->type() == TokenType::True || token->type() == TokenType::False) {
        if (token->type() == TokenType::True) {
            m_value = llvm::ConstantInt::get(type, 1);
        } else if (token->type() == TokenType::False) {
            m_value = llvm::ConstantInt::get(type, 0);
        } else {
            if (std::stod(lexeme) == 0.0) {
                m_value = llvm::ConstantInt::get(type, 0);
            } else {
                m_value = llvm::ConstantInt::get(type, 1);
            }
        }
    } else if (local->isIntegral()) {
        m_value = llvm::ConstantInt::get(llvm::cast<llvm::IntegerType>(type), lexeme, 10);
    } else if (ast->type->isFloatingPoint()) {
        m_value = llvm::ConstantFP::get(type, lexeme);
    } else if (local->isPointer()) {
        if (local->IsAnyPtr()) {
            m_value = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(type));
        } else {
            auto llvmType = llvm::cast<llvm::IntegerType>(PrimitiveType::get(TokenType::LongInt)->llvm());
            auto constant = llvm::ConstantInt::get(llvmType, lexeme, 10);
            m_value = new llvm::IntToPtrInst(constant, type, "", m_block);
        }
    }
}


//
// AstVarDecl
void IrBuilder::visit(AstVarDecl * ast)
{
    const string & id = ast->id->token->lexeme();
    m_lastId = id;
    
    auto sym = m_table->get(id);
    auto llType = sym->type()->llvm();
    // if m_block is null then is a global variable
    if (m_block == nullptr) {
        llvm::Constant * constant;
        if (llType->isPointerTy()) {
            constant = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(llType));
        } else if (llType->isIntegerTy()) {
            constant = llvm::ConstantInt::get(llType, 0, false);
        } else if (llType->isFloatingPointTy()) {
            constant = llvm::ConstantFP::get(llType, 0);
        }
        sym->value = new llvm::GlobalVariable(
            *m_module,
            llType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            constant,
            id
        );
    } else {
        sym->value = new llvm::AllocaInst(llType, id, m_block);
    }
}


//
// AstAssignStmt
void IrBuilder::visit(AstAssignStmt * ast)
{
    // left value
    llvm::Value * dst = nullptr;
    
    // dereference ?
    if (ast->left->is(Ast::DereferenceExpr)) {
        static_cast<AstDereferenceExpr *>(ast->left.get())->expr->accept(this);
        dst = m_value;
    } else {
        dst = m_table->get(static_cast<AstIdentExpr *>(ast->left.get())->token->lexeme())->value;
        m_lastId = static_cast<AstIdentExpr *>(ast->left.get())->token->lexeme();
    }
    
    // right hand expression
    ast->right->accept(this);
    
    // store nstr
    new llvm::StoreInst(m_value, dst, m_block);
}


//
// AstAddressOfExpr
void IrBuilder::visit(AstAddressOfExpr * ast)
{
    const string & id = ast->id->token->lexeme();
    m_value = m_table->get(id)->value;
    m_lastId = id;
}


//
// AstDereferenceExpr
void IrBuilder::visit(AstDereferenceExpr * ast)
{
    ast->expr->accept(this);
    m_value = new llvm::LoadInst(m_value, "", m_block);
}


//
//
void IrBuilder::visit(AstBinaryExpr * ast)
{
    // left
    ast->lhs->accept(this);
    auto left = m_value;
    // right
    ast->rhs->accept(this);
    auto right = m_value;
    // resulting type
    
    // integer types
    if (ast->lhs->type->isIntegral() || ast->lhs->type->isPointer()) {
        llvm::CmpInst::Predicate pred;
        auto local = ast->token->type();
        if (local == TokenType::Equal)          pred = llvm::CmpInst::Predicate::ICMP_EQ;
        else if (local == TokenType::NotEqual)  pred = llvm::CmpInst::Predicate::ICMP_NE;
        
        m_value = new llvm::ICmpInst(*m_block, pred, left, right, "");
    }
    // fp types
    else if (ast->lhs->type->isFloatingPoint())
    {
        llvm::CmpInst::Predicate pred;
        auto local = ast->token->type();
        if (local == TokenType::Equal)          pred = llvm::CmpInst::Predicate::FCMP_OEQ;
        else if (local == TokenType::NotEqual)  pred = llvm::CmpInst::Predicate::FCMP_UNE;
        
        m_value = new llvm::FCmpInst(*m_block, pred, left, right, "");
    }
    
    // cast i1 to i8
    // get cast opcode
    auto opcode = llvm::CastInst::getCastOpcode(
        m_value,
        false,
        ast->type->llvm(),
        false
    );
    
    // create cast instruction
    m_value = llvm::CastInst::Create(opcode, m_value, ast->type->llvm(), "", m_block);
}


//
// AstCastExpr
void IrBuilder::visit(AstCastExpr * ast)
{
    ast->expr->accept(this);
    auto src = ast->expr->type;
    auto dst = ast->type;
    bool srcSigned = src->isSignedIntegral();
    
    // if target is boolean
    // then comparison is required
    if (dst->isBoolean()) {
        if (src->isIntegral()) {
            auto const_int = llvm::ConstantInt::get(src->llvm(), 0);
            m_value = new llvm::ICmpInst(*m_block, llvm::ICmpInst::ICMP_NE, m_value, const_int, "");
            srcSigned = false;
        } else if (src->isFloatingPoint()) {
            auto const_fp = llvm::ConstantFP::get(src->llvm(), 0);
            m_value = new llvm::FCmpInst(*m_block, llvm::ICmpInst::FCMP_UNE, m_value, const_fp, "");
            srcSigned = false;
        } else if (src->isPointer()) {
            auto const_null = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(src->llvm()));
            m_value = new llvm::ICmpInst(*m_block, llvm::ICmpInst::ICMP_NE, m_value, const_null, "");
            srcSigned = false;
        }
    }
    
    // get cast opcode
    auto opcode = llvm::CastInst::getCastOpcode(
        m_value,
        srcSigned,
        dst->llvm(),
        dst->isSignedIntegral()
    );
    
    // create cast instruction
    m_value = llvm::CastInst::Create(opcode, m_value, dst->llvm(), "", m_block);
}


//
// AstCallExpr
void IrBuilder::visit(AstCallExpr * ast)
{
    
    const string & id = ast->id->token->lexeme();
    auto sym = m_table->get(id);
    
    vector<llvm::Value *> args;
    if (ast->args) {
        SCOPED_GUARD(m_lastId);
        for (auto & arg : ast->args->args) {
            arg->accept(this);
            args.push_back(m_value);
        }
    }
    
    m_value = llvm::CallInst::Create(sym->value, args, m_lastId, m_block);
}


//
// AstIdentExpr
void IrBuilder::visit(AstIdentExpr * ast)
{
    const string & id = ast->token->lexeme();
    m_value = new llvm::LoadInst(m_table->get(id)->value, "", m_block);
    m_lastId = id;
}


//
// AstCallStmt
void IrBuilder::visit(AstCallStmt * ast)
{
    m_lastId = "";
    ast->expr->accept(this);
}


//
// AstIfStmt
void IrBuilder::visit(AstIfStmt * ast)
{
    m_lastId = "";
    SCOPED_GUARD(m_endIfBlock);
    SCOPED_GUARD(m_isElseIf);
    
    // expression
    ast->expr->accept(this);
    
    // cast to i1
    m_value = new llvm::TruncInst(m_value, llvm::Type::getInt1Ty(m_module->getContext()), "", m_block);
    
    // true block
    llvm::BasicBlock * trueBlock = llvm::BasicBlock::Create(m_module->getContext(), "", m_function, m_endIfBlock);
    
    // else block
    llvm::BasicBlock * falseBlock = ast->falseBlock
                                  ? llvm::BasicBlock::Create(m_module->getContext(), "", m_function, m_endIfBlock)
                                  : nullptr;
    // end block
    if (m_isElseIf) {
        m_isElseIf = false;
    } else {
        m_endIfBlock = llvm::BasicBlock::Create(m_module->getContext(), "", m_function, m_endIfBlock);
    }
    
    // create branch instruction
    llvm::BranchInst::Create(trueBlock, falseBlock ? falseBlock : m_endIfBlock, m_value, m_block);
    
    // process true block
    m_block = trueBlock;
    ast->trueBlock->accept(this);
    llvm::BranchInst::Create(m_endIfBlock, m_block);
    
    // process falseBlock
    if (falseBlock) {
        if (ast->falseBlock->is(Ast::IfStmt)) m_isElseIf = true;
        m_block = falseBlock;
        ast->falseBlock->accept(this);
        if (!m_isElseIf) llvm::BranchInst::Create(m_endIfBlock, m_block);
    }
    
    // set end block as the new block
    if (!m_isElseIf) m_block = m_endIfBlock;
}







