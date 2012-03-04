//
//  CodeGen.cpp
//  LightBASIC
//
//  Created by Albert Varaksin on 03/03/2012.
//  Copyright (c) 2012 LightBASIC development team. All rights reserved.
//
#include "CodeGen.h"
#include "Token.h"
#include "Ast.h"
#include "Type.h"
#include "Symbol.h"
#include "SymbolTable.h"
#include "stdint.h"

// llvm
#include "llvm/DerivedTypes.h"
#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/IRBuilder.h"

using namespace lbc;

//
// create
CodeGen::CodeGen() : m_module(nullptr), m_table(nullptr), m_function(nullptr)
{
}


//
// AstDeclList
void CodeGen::visit(AstDeclList * ast)
{
	m_module = new llvm::Module("app", llvm::getGlobalContext());
	m_table = ast->symbolTable.get();
	for (auto & decl : ast->decls) decl.accept(this);
	m_module->dump();
}



//
// generate llvm type from local type
llvm::Type * getType(const shared_ptr<Type> & local, llvm::LLVMContext & context) {
	
	llvm::Type * llvmType = nullptr;
	
	// pointer
	if (local->kind() == Type::Ptr) {
		auto ptr = static_pointer_cast<PtrType>(local);
		auto base = ptr->getBaseType();
		auto llType = llvm::Type::getIntNPtrTy(context, base->getSizeInBits());
		int level = ptr->indirection() - 1;
		while(level--) {
			llType = llType->getPointerTo();
		}
		llvmType = llType;
	}
	// basic type
	else if (local->kind() == Type::Basic) {
		llvmType = llvm::Type::getIntNTy(context, local->getSizeInBits());
	}
	// function type
	else if (local->kind() == Type::Function) {
		std::vector<llvm::Type*> params;
		auto fnType = static_pointer_cast<FunctionType>(local);
		for (auto p : fnType->params) {
			params.push_back(getType(p, context));
		}
		llvmType = llvm::FunctionType::get(
			getType(fnType->result(), context),
			params,
			false
		);
	}
	// nothing
	local->llvmType = llvmType;
	return llvmType;
}


//
// AstFuncSignature
void CodeGen::visit(AstFuncSignature * ast)
{
	// the id
	const string & id = ast->id->token->lexeme();
	
	// get the function
	auto fn = m_module->getFunction(id);
	if (fn) return;
	
	// get the symbol
	auto sym = m_table->get(id);
	auto llvmType = getType(sym->type(), m_module->getContext());
	m_function = llvm::Function::Create(
		 llvm::cast<llvm::FunctionType>(llvmType),
		 llvm::GlobalValue::ExternalLinkage,
		 id == "PRINT" ? "puts" : id, // temporary HACK. until attributes are properly proccessed
		 m_module
	);
	m_function->setCallingConv(llvm::CallingConv::C);
}


//
// AstFunctionDecl
void CodeGen::visit(AstFunctionDecl * ast)
{	
	// process the signature
	ast->signature->accept(this);
}


//
// AstFunctionStmt
void CodeGen::visit(AstFunctionStmt * ast)
{
	// function signature
	ast->signature->accept(this);
	
	// symbol table
	m_table = ast->stmts->symbolTable.get();
	
	// declare function parameters as local variables? how to access them?
	
	// create the block
	{
		llvm::BasicBlock::Create(m_module->getContext(), "", m_function, 0);
		ast->stmts->accept(this);
	}
	
	// restore
	m_table = m_table->parent();
}


//
// AstVarDecl
void CodeGen::visit(AstVarDecl * ast)
{
	
}


//
// AstAssignStmt
void CodeGen::visit(AstAssignStmt * ast)
{
	
}


//
// AstLiteralExpr
void CodeGen::visit(AstLiteralExpr * ast)
{
	
}


//
// AstCallStmt
void CodeGen::visit(AstCallStmt * ast)
{
	
}


//
// AstCallExpr
void CodeGen::visit(AstCallExpr * ast)
{
	
}


//
// AstIdentExpr
void CodeGen::visit(AstIdentExpr * ast)
{
	
}



//
// AstReturnStmt
void CodeGen::visit(AstReturnStmt * ast)
{
	
}
