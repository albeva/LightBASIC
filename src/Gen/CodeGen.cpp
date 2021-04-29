//
// Created by Albert Varaksin on 05/07/2020.
//
#include "CodeGen.h"
#include "Ast/Ast.h"
#include "Driver/Context.h"
#include "Lexer/Token.h"
#include "Symbol/Symbol.h"
#include "Symbol/SymbolTable.h"
#include "Type/Type.h"
#include <charconv>
#include <llvm/IR/IRPrintingPasses.h>
using namespace lbc;

CodeGen::CodeGen(Context& context)
: m_context{ context },
  m_llvmContext{ context.getLlvmContext() },
  m_builder{ m_llvmContext } {
}

bool CodeGen::validate() const {
    assert(m_module != nullptr); // NOLINT
    return !llvm::verifyModule(*m_module, &llvm::outs());
}

void CodeGen::print() const {
    assert(m_module != nullptr); // NOLINT
    auto* printer = llvm::createPrintModulePass(llvm::outs());
    printer->runOnModule(*m_module);
}

void CodeGen::visit(AstModule* ast) {
    m_astRootModule = ast;
    m_fileId = ast->fileId;
    m_scope = Scope::Root;
    auto file = m_context.getSourceMrg().getMemoryBuffer(m_fileId)->getBufferIdentifier();

    m_module = make_unique<llvm::Module>(file, m_llvmContext);
    m_module->setTargetTriple(m_context.getTriple().str());

    if (ast->hasImplicitMain) {
        m_function = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(m_llvmContext), false),
            llvm::Function::ExternalLinkage,
            "main",
            *m_module);
        m_function->setCallingConv(llvm::CallingConv::C);
        m_function->setDSOLocal(true);
        m_function->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Local);
        m_block = llvm::BasicBlock::Create(m_llvmContext, "", m_function);
    } else {
        m_block = llvm::BasicBlock::Create(m_llvmContext);
    }
    m_builder.SetInsertPoint(m_block);

    // parse statements
    visitStmtList(ast->stmtList.get());

    // close main
    if (ast->hasImplicitMain) {
        auto* retValue = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(m_llvmContext));
        llvm::ReturnInst::Create(m_module->getContext(), retValue, m_block);
    }
}

void CodeGen::visitStmtList(AstStmtList* ast) {
    for (const auto& stmt : ast->stmts) {
        visitStmt(stmt.get());
    }
}

void CodeGen::visitAssignStmt(AstAssignStmt* ast) {
    auto* dstValue = getStoreValue(ast->identExpr.get());
    visitExpr(ast->expr.get());
    m_builder.CreateStore(ast->expr->llvmValue, dstValue);
}

llvm::Value* CodeGen::getStoreValue(AstIdentExpr* identExpr) {
    return identExpr->symbol->value();
}

void CodeGen::visitExprStmt(AstExprStmt* ast) {
    visitExpr(ast->expr.get());
}

void CodeGen::visitVarDecl(AstVarDecl* ast) {
    // process resulting type
    visitTypeExpr(ast->typeExpr.get());

    llvm::Constant* exprValue = nullptr;
    llvm::Type* exprType = nullptr;
    bool generateStoreInCtror = false;

    // has an init expr?
    if (ast->expr) {
        visitExpr(ast->expr.get());
        if (auto* constExpr = dyn_cast<llvm::Constant>(ast->expr->llvmValue)) {
            exprValue = constExpr;
            exprType = exprValue->getType();
        } else {
            exprType = ast->expr->type->llvmType(m_llvmContext);
            exprValue = llvm::Constant::getNullValue(exprType);
            generateStoreInCtror = true;
        }
    } else {
        exprType = ast->typeExpr->type->llvmType(m_llvmContext);
        exprValue = llvm::Constant::getNullValue(exprType);
    }

    llvm::Value* value;
    if (m_scope == Scope::Root) {
        value = new llvm::GlobalVariable(
            *m_module,
            exprType,
            false,
            llvm::GlobalValue::PrivateLinkage,
            exprValue,
            ast->symbol->identifier());
        if (generateStoreInCtror) {
            new llvm::StoreInst(ast->expr->llvmValue, value, m_block);
        }
    } else {
        llvm::IRBuilder<> builder(m_block);
        value = new llvm::AllocaInst(
            exprType,
            0,
            "",
            m_block);
        new llvm::StoreInst(exprValue, value, m_block);
    }

    ast->symbol->setValue(value);
}

void CodeGen::visitFuncDecl(AstFuncDecl* ast) {
    auto* fnTy = llvm::cast<llvm::FunctionType>(ast->symbol->type()->llvmType(m_llvmContext));
    auto* fn = llvm::Function::Create(
        fnTy,
        llvm::GlobalValue::ExternalLinkage,
        ast->symbol->identifier(),
        *m_module);
    fn->setCallingConv(llvm::CallingConv::C);
    ast->symbol->setValue(fn);
}

void CodeGen::visitFuncParamDecl(AstFuncParamDecl* /*ast*/) {
}

void CodeGen::visitFuncStmt(AstFuncStmt* ast) {
    visitFuncDecl(ast->decl.get());
    RESTORE_ON_EXIT(m_function);
    RESTORE_ON_EXIT(m_scope);
    m_scope = Scope::Function;
    m_function = llvm::cast<llvm::Function>(ast->decl->symbol->value());
    m_block = llvm::BasicBlock::Create(m_llvmContext, "", m_function);

    visitStmtList(ast->stmtList.get());

    if (!m_block->getTerminator()) {
        llvm::ReturnInst::Create(m_llvmContext, nullptr, m_block);
    }
}

void CodeGen::visitAttributeList(AstAttributeList* /*ast*/) {
}

void CodeGen::visitAttribute(AstAttribute* /*ast*/) {
}

void CodeGen::visitIdentExpr(AstIdentExpr* ast) {
    const auto* type = ast->type;
    if (isa<TypeFunction>(type)) {
        ast->llvmValue = ast->symbol->value();
        return;
    }

    auto* sym = ast->symbol;
    ast->llvmValue = new llvm::LoadInst(
        sym->type()->llvmType(m_llvmContext),
        sym->value(),
        "",
        m_block);
}

void CodeGen::visitTypeExpr(AstTypeExpr* /*ast*/) {
}

void CodeGen::visitCallExpr(AstCallExpr* ast) {
    visitIdentExpr(ast->identExpr.get());

    auto* fn = llvm::cast<llvm::Function>(ast->identExpr->llvmValue);

    std::vector<llvm::Value*> args;
    args.reserve(ast->argExprs.size());
    for (const auto& arg : ast->argExprs) {
        visitExpr(arg.get());
        args.emplace_back(arg->llvmValue);
    }

    auto* inst = llvm::CallInst::Create(llvm::FunctionCallee(fn), args, "", m_block);
    inst->setTailCall(false);
}

void CodeGen::visitLiteralExpr(AstLiteralExpr* ast) {
    llvm::Constant* constant = nullptr;
    const auto& lexeme = ast->token->lexeme();

    switch (ast->token->kind()) {
    case TokenKind::StringLiteral: {
        auto iter = m_stringLiterals.find(lexeme);
        if (iter != m_stringLiterals.end()) {
            constant = iter->second;
        } else {
            constant = llvm::ConstantDataArray::getString(
                m_llvmContext,
                lexeme,
                true);

            auto* value = new llvm::GlobalVariable(
                *m_module,
                constant->getType(),
                true,
                llvm::GlobalValue::PrivateLinkage,
                constant,
                ".str");
            value->setAlignment(llvm::MaybeAlign(1));
            value->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Local);

            llvm::Constant* zero_32 = llvm::Constant::getNullValue(
                llvm::IntegerType::getInt32Ty(m_llvmContext));

            std::array indices{
                zero_32,
                zero_32
            };

            constant = llvm::ConstantExpr::getGetElementPtr(nullptr, value, indices, true);
            m_stringLiterals.insert({lexeme, constant});
        }
        break;
    }
    case TokenKind::NumberLiteral: {
        uint64_t result = 0;
        std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), result); // NOLINT
        constant = llvm::ConstantInt::get(
            ast->type->llvmType(m_llvmContext),
            result,
            llvm::cast<TypeNumeric>(ast->type)->isSigned());
        break;
    }
    case TokenKind::BooleanLiteral: {
        uint64_t value = lexeme == "TRUE" ? 1 : 0;
        constant = llvm::ConstantInt::get(
            ast->type->llvmType(m_llvmContext),
            value,
            llvm::cast<TypeNumeric>(ast->type)->isSigned());
        break;
    }
    case TokenKind::NullLiteral:
        constant = llvm::ConstantPointerNull::get(
            llvm::cast<llvm::PointerType>(ast->type->llvmType(m_llvmContext)));
        break;
    default:
        fatalError("Invalid literal type");
    }

    ast->llvmValue = constant;
}

unique_ptr<llvm::Module> CodeGen::getModule() {
    return std::move(m_module);
}
