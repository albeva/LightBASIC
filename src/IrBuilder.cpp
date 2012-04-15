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
#include "Context.h"
#include <iostream>


using namespace lbc;

//
// create
IrBuilder::IrBuilder(Context & ctx)
:   m_module(nullptr),
    m_function(nullptr),
    m_block(nullptr),
    m_edgeBlock(nullptr),
    m_value(nullptr),
    m_table(nullptr),
    m_isElseIf(false),
    m_ctx(ctx)
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
    m_edgeBlock = nullptr;
    m_value = nullptr;
    m_table = nullptr;
    m_isElseIf = false;
    
    // the module
    m_module = new llvm::Module(ast->name, llvm::getGlobalContext());
    
    // set stuff
//#if defined __linux__
//    if (m_ctx.arch() == Architecture::X86_32) {
//        m_module->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32");
//        m_module->setTargetTriple("i386-pc-linux-gnu");
//    } else if (m_ctx.arch() == Architecture::X86_64) {
//        m_module->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64");
//        m_module->setTargetTriple("x86_64-pc-linux-gnu");
//    }    
//#elif defined __APPLE__
//    if (m_ctx.arch() == Architecture::X86_32) {
//        m_module->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:32:64-f32:32:32-f64:32:64-v64:64:64-v128:128:128-a0:0:64-f80:128:128-n8:16:32-S128");
//        m_module->setTargetTriple("i386-apple-macosx10.6");
//    } else if (m_ctx.arch() == Architecture::X86_64) {
//        m_module->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128");
//        m_module->setTargetTriple("x86_64-apple-macosx10.6");
//    }
//#else
//    #error Unsupported system
//#endif
    auto & triple = m_ctx.triple();
    m_module->setTargetTriple(triple.getTriple());
    
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
    for (auto & stmt : ast->stmts) {
        if (m_block->getTerminator()) {
            m_block = llvm::BasicBlock::Create(m_module->getContext(), "", m_function, 0);
        }
        stmt->accept(this);
    }
}


//
// AstFuncSignature
void IrBuilder::visit(AstFuncSignature * ast)
{
    // the id
    const std::string & id = ast->id->token->lexeme();
    Symbol * sym = m_table->get(id);
    
    // get the function
    if (!sym->value) {
        assert(!m_module->getFunction(id));
        // get the symbol
        auto llvmType = sym->type()->llvm();
        std::string alias = sym->alias();
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
    
    // add return if this is a SUB
    if (!ast->signature->typeExpr) {
        if (!m_block->getTerminator()) {
            llvm::ReturnInst::Create(m_module->getContext(), nullptr, m_block);
        }
    }
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
    const std::string & lexeme = ast->token->lexeme();
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
                llvm::ConstantDataArray::getString(context, lexeme, true),
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
    const std::string & id = ast->id->token->lexeme();
    m_lastId = id;
    
    auto sym = m_table->get(id);
    auto llType = sym->type()->llvm();
    // if m_block is null then is a global variable
    if (m_block == nullptr) {
        llvm::Constant * constant = nullptr;
        if (llType->isPointerTy()) {
            constant = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(llType));
        } else if (llType->isIntegerTy()) {
            constant = llvm::ConstantInt::get(llType, 0, false);
        } else if (llType->isFloatingPointTy()) {
            constant = llvm::ConstantFP::get(llType, 0);
        } else {
            return; // TODO raise error!
        }
        sym->value = new llvm::GlobalVariable(
            *m_module,
            llType,
            false,
            llvm::GlobalValue::ExternalLinkage,
            constant,
            id
        );
        m_value = sym->value ;
    } else {
        sym->value = new llvm::AllocaInst(llType, id, m_block);
        if (ast->expr) {
            ast->expr->accept(this);
            new llvm::StoreInst(m_value, sym->value, m_block);
        }
        m_value = sym->value;
    }
}


//
// AstAssignStmt
void IrBuilder::visit(AstAssignStmt * ast)
{    
    // right hand expression
    ast->right->accept(this);
    auto src = m_value;
    
    // dereference ?
    if (ast->left->is(Ast::DereferenceExpr)) {
        static_cast<AstDereferenceExpr *>(ast->left.get())->expr->accept(this);
    } else {
        m_value = m_table->get(static_cast<AstIdentExpr *>(ast->left.get())->token->lexeme())->value;
        m_lastId = static_cast<AstIdentExpr *>(ast->left.get())->token->lexeme();
    }
    
    // store nstr
    new llvm::StoreInst(src, m_value, m_block);
}


//
// AstAddressOfExpr
void IrBuilder::visit(AstAddressOfExpr * ast)
{
    const std::string & id = ast->id->token->lexeme();
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
    
    // integer types
    auto type = ast->lhs->type;
    
    // generate the instruction
    m_value = emitBinaryExpr(left, right, ast->token->type(), type);
    
    // expand to 8bit if is boolean and int1
    if (m_value->getType()->isIntegerTy(1)) {
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
}


//
// AstCastExpr
void IrBuilder::visit(AstCastExpr * ast)
{
    ast->expr->accept(this);
    auto src = ast->expr->type;
    auto dst = ast->type;
    m_value = emitCastExpr(m_value, src, dst);
}



/**
 * emit binary expression instruction
 */
llvm::Value * IrBuilder::emitBinaryExpr(llvm::Value * left, llvm::Value * right, TokenType op, Type * type)
{
    if (op == TokenType::Modulus) {
        llvm::Instruction::BinaryOps instr;
        if (type->isSignedIntegral()) {
            instr = llvm::Instruction::SRem;
        } else {
            instr = llvm::Instruction::URem;
        }
        return llvm::BinaryOperator::Create(instr, left, right, "", m_block);        
    } else {
        if (type->isIntegral() || type->isPointer()) {
            auto isgned = type->isSignedIntegral();
            llvm::CmpInst::Predicate pred;
            if (op == TokenType::Equal)                 pred = llvm::CmpInst::Predicate::ICMP_EQ;
            else if (op == TokenType::NotEqual)         pred = llvm::CmpInst::Predicate::ICMP_NE;
            else if (op == TokenType::LessThanEqual)    pred = isgned ? llvm::CmpInst::Predicate::ICMP_SLE : llvm::CmpInst::Predicate::ICMP_ULE;
            else if (op == TokenType::GreaterThanEqual) pred = isgned ? llvm::CmpInst::Predicate::ICMP_SGE : llvm::CmpInst::Predicate::ICMP_UGE;
            else if (op == TokenType::LessThan)         pred = isgned ? llvm::CmpInst::Predicate::ICMP_SLT : llvm::CmpInst::Predicate::ICMP_ULT;
            else if (op == TokenType::GreaterThan)      pred = isgned ? llvm::CmpInst::Predicate::ICMP_SGT : llvm::CmpInst::Predicate::ICMP_UGT;            
            else {
                return nullptr;
            }
            return new llvm::ICmpInst(*m_block, pred, left, right, "");
        }
        // fp types
        else if (type->isFloatingPoint())
        {
            llvm::CmpInst::Predicate pred;
            if (op == TokenType::Equal)                 pred = llvm::CmpInst::Predicate::FCMP_OEQ;
            else if (op == TokenType::NotEqual)         pred = llvm::CmpInst::Predicate::FCMP_UNE;
            else if (op == TokenType::LessThanEqual)    pred = llvm::CmpInst::Predicate::FCMP_OLE;
            else if (op == TokenType::GreaterThanEqual) pred = llvm::CmpInst::Predicate::FCMP_OGE;
            else if (op == TokenType::LessThan)         pred = llvm::CmpInst::Predicate::FCMP_OLT;
            else if (op == TokenType::GreaterThan)      pred = llvm::CmpInst::Predicate::FCMP_OGT;
            else {
                return nullptr;
            }
           return new llvm::FCmpInst(*m_block, pred, left, right, "");
        }
    }
    THROW_EXCEPTION("Invalid types");
    return nullptr;
}


/**
 * Emit cast instruction
 */
llvm::Value * IrBuilder::emitCastExpr(llvm::Value * value, Type * src, Type * dst)
{
    bool srcSigned = src->isSignedIntegral();
    
    // if target is boolean
    // then comparison is required
    if (dst->isBoolean()) {
        if (src->isIntegral()) {
            auto const_int = llvm::ConstantInt::get(src->llvm(), 0);
            value = new llvm::ICmpInst(*m_block, llvm::ICmpInst::ICMP_NE, value, const_int, "");
            srcSigned = false;
        } else if (src->isFloatingPoint()) {
            auto const_fp = llvm::ConstantFP::get(src->llvm(), 0);
            value = new llvm::FCmpInst(*m_block, llvm::ICmpInst::FCMP_UNE, value, const_fp, "");
            srcSigned = false;
        } else if (src->isPointer()) {
            auto const_null = llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(src->llvm()));
            value = new llvm::ICmpInst(*m_block, llvm::ICmpInst::ICMP_NE, value, const_null, "");
            srcSigned = false;
        }
    }
    
    // get cast opcode
    auto opcode = llvm::CastInst::getCastOpcode(
        value,
        srcSigned,
        dst->llvm(),
        dst->isSignedIntegral()
    );
    
    // create cast instruction
    return llvm::CastInst::Create(opcode, value, dst->llvm(), "", m_block);
}




//
// AstCallExpr
void IrBuilder::visit(AstCallExpr * ast)
{
    
    const std::string & id = ast->id->token->lexeme();
    auto sym = m_table->get(id);
    
    std::vector<llvm::Value *> args;
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
    const std::string & id = ast->token->lexeme();
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
    SCOPED_GUARD(m_edgeBlock);
    SCOPED_GUARD(m_isElseIf);
    
    // expression
    ast->expr->accept(this);
    
    // cast to i1
    m_value = new llvm::TruncInst(m_value, llvm::Type::getInt1Ty(m_module->getContext()), "", m_block);
    
    // true block
    auto trueBlock  = llvm::BasicBlock::Create(m_module->getContext(), "", m_function, m_edgeBlock);
    auto falseBlock = ast->falseBlock
                    ? llvm::BasicBlock::Create(m_module->getContext(), "", m_function, m_edgeBlock)
                    : nullptr;
    // end block
    if (m_isElseIf) {
        m_isElseIf = false;
    } else {
        m_edgeBlock = llvm::BasicBlock::Create(m_module->getContext(), "", m_function, m_edgeBlock);
    }
    
    // create branch instruction
    llvm::BranchInst::Create(trueBlock, falseBlock ? falseBlock : m_edgeBlock, m_value, m_block);
    
    // process true block
    m_block = trueBlock;
    ast->trueBlock->accept(this);
    if (!m_block->getTerminator()) {
        llvm::BranchInst::Create(m_edgeBlock, m_block);
    }
    
    // process falseBlock
    if (falseBlock) {
        if (ast->falseBlock->is(Ast::IfStmt)) m_isElseIf = true;
        m_block = falseBlock;
        ast->falseBlock->accept(this);
        if (!m_isElseIf && !m_block->getTerminator()) {
            llvm::BranchInst::Create(m_edgeBlock, m_block);
        }
    }
    
    // set end block as the new block
    if (!m_isElseIf) m_block = m_edgeBlock;
}


//
// AstForStmt
void IrBuilder::visit(AstForStmt * ast)
{
    SCOPED_GUARD(m_edgeBlock);
    SCOPED_GUARD(m_table);
    m_table = ast->block->symbolTable;
    
    // blocks
    auto for_head = llvm::BasicBlock::Create(m_module->getContext(), "for_head", m_function, m_edgeBlock);
    auto for_body = llvm::BasicBlock::Create(m_module->getContext(), "for_body", m_function, m_edgeBlock);
    auto for_foot = llvm::BasicBlock::Create(m_module->getContext(), "for_foot", m_function, m_edgeBlock);
    auto for_exit = llvm::BasicBlock::Create(m_module->getContext(), "for_exit", m_function, m_edgeBlock);
    
    // initialize the variable
    ast->stmt->accept(this);
    auto alloc = m_value;
    // jump to head
    llvm::BranchInst::Create(for_head, m_block);
    m_block = for_head;
    m_edgeBlock = for_body;
    
    // load the var
    auto value = new llvm::LoadInst(alloc, "", m_block);

    // load the end
    ast->end->accept(this);
    auto end = m_value;
    auto type = ast->end->type; // should get from the value !

    // compare
    m_value = emitBinaryExpr(value, end, TokenType::LessThanEqual, type);
    llvm::BranchInst::Create(for_body, for_exit, m_value, m_block);
    m_block = for_body;
    m_edgeBlock = for_foot;
    
    // the body
    ast->block->accept(this);
    if (!m_block->getTerminator()) {
        llvm::BranchInst::Create(for_foot, m_block);
    }
    m_block = for_foot;
    m_edgeBlock = for_exit;
    
    // the footer
    if (ast->step) {
        ast->step->accept(this);
    } else {
        if (type->isIntegral()) {
            m_value = llvm::ConstantInt::get(type->llvm(), 1, type->isSignedIntegral());
        } else if (type->isFloatingPoint()) {
            m_value = llvm::ConstantFP::get(type->llvm(), 1.0);
        } else {
            THROW_EXCEPTION(std::string("Unsupported type in FOR: ") + type->toString());
        }
    }
    
    // ADD instruction
    m_value = llvm::BinaryOperator::Create(llvm::Instruction::Add, value, m_value, "", m_block);
    new llvm::StoreInst(m_value, alloc, m_block);
    llvm::BranchInst::Create(for_head, m_block);
    
    // done
    m_block = for_exit;
}





