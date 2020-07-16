//
// Created by Albert on 05/07/2020.
//
#include "CodeGen.h"
#include "Ast/Ast.h"
#include "Lexer/Token.h"
#include "Symbol/Symbol.h"
#include "Symbol/SymbolTable.h"
#include "Type/Type.h"
#include <charconv>
#include <llvm/IR/IRPrintingPasses.h>
using namespace lbc;

[[noreturn]] static void error(const string& message) {
    std::cerr << message << std::endl;
    std::exit(EXIT_FAILURE);
}

CodeGen::CodeGen(llvm::LLVMContext& context, llvm::SourceMgr& srcMgr, llvm::Triple& tripe, unsigned fileId)
: m_context{ context },
  m_srcMgr{ srcMgr },
  m_tripe{ tripe },
  m_fileId{ fileId },
  m_builder{ context } {}

void CodeGen::visit(AstProgram* ast) {
    auto file = m_srcMgr.getMemoryBuffer(m_fileId)->getBufferIdentifier();

    m_module = make_unique<llvm::Module>(file, m_context);
    m_module->setTargetTriple(m_tripe.str());

    // main
    m_function = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getInt32Ty(m_context), false),
        llvm::Function::ExternalLinkage,
        "main",
        *m_module);
    m_function->setCallingConv(llvm::CallingConv::C);
    m_function->setDSOLocal(true);
    m_function->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Local);
    m_block = llvm::BasicBlock::Create(m_context, "", m_function);

    m_builder.SetInsertPoint(m_block);

    // parse statements
    ast->stmtList->accept(this);

    // close main
    auto* retValue = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(m_context));
    llvm::ReturnInst::Create(m_module->getContext(), retValue, m_block);
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

void CodeGen::visit(AstStmtList* ast) {
    for (const auto& stmt : ast->stmts) {
        stmt->accept(this);
    }
}

void CodeGen::visit(AstAssignStmt* ast) {
    auto* dstValue = getStoreValue(ast->identExpr.get());
    ast->expr->accept(this);
    m_builder.CreateStore(ast->expr->llvmValue, dstValue);
}

llvm::Value* CodeGen::getStoreValue(AstIdentExpr* identExpr) {
    return identExpr->symbol->value();
}

void CodeGen::visit(AstExprStmt* ast) {
    ast->expr->accept(this);
}

void CodeGen::visit(AstVarDecl* ast) {
    // process resulting type
    ast->typeExpr->accept(this);

    llvm::Constant* exprValue = nullptr;
    llvm::Type* exprType = nullptr;
    bool generateStoreInCtror = false;

    // has an init expr?
    if (ast->expr) {
        ast->expr->accept(this);
        if (auto* constExpr = dyn_cast<llvm::Constant>(ast->expr->llvmValue)) {
            exprValue = constExpr;
            exprType = exprValue->getType();
        } else {
            exprType = ast->expr->type->llvmType(m_context);
            exprValue = llvm::Constant::getNullValue(exprType);
            generateStoreInCtror = true;
        }
    } else {
        exprType = ast->typeExpr->type->llvmType(m_context);
        exprValue = llvm::Constant::getNullValue(exprType);
    }

    auto* value = new llvm::GlobalVariable(
        *m_module,
        exprType,
        false,
        llvm::GlobalValue::PrivateLinkage,
        exprValue,
        view_to_stringRef(ast->symbol->name()));

    if (generateStoreInCtror) {
        new llvm::StoreInst(ast->expr->llvmValue, value, m_block);
    }

    ast->symbol->setValue(value);
}

void CodeGen::visit(AstFuncDecl* ast) {
    auto* fnTy = llvm::cast<llvm::FunctionType>(ast->symbol->type()->llvmType(m_context));
    auto* fn = llvm::Function::Create(
        fnTy,
        llvm::GlobalValue::ExternalLinkage,
        view_to_stringRef(ast->symbol->alias()),
        *m_module);
    fn->setCallingConv(llvm::CallingConv::C);
    ast->symbol->setValue(fn);
}

void CodeGen::visit(AstFuncParamDecl* ast) {
}

void CodeGen::visit(AstAttributeList* ast) {
}

void CodeGen::visit(AstAttribute* ast) {
}

void CodeGen::visit(AstIdentExpr* ast) {
    const auto* type = ast->type;
    if (isa<TypeFunction>(type)) {
        ast->llvmValue = ast->symbol->value();
        return;
    }
    ast->llvmValue = new llvm::LoadInst(ast->symbol->value(), "", m_block); // NOLINT
}

void CodeGen::visit(AstTypeExpr* ast) {
}

void CodeGen::visit(AstCallExpr* ast) {
    ast->identExpr->accept(this);

    auto* fn = llvm::cast<llvm::Function>(ast->identExpr->llvmValue);

    std::vector<llvm::Value*> args;
    args.reserve(ast->argExprs.size());
    for (const auto& arg : ast->argExprs) {
        arg->accept(this);
        args.emplace_back(arg->llvmValue);
    }

    auto* inst = llvm::CallInst::Create(llvm::FunctionCallee(fn), args, "", m_block);
    inst->setTailCall(false);
}

void CodeGen::visit(AstLiteralExpr* ast) {
    llvm::Constant* constant = nullptr;
    const auto& lexeme = ast->token->lexeme();

    switch (ast->token->kind()) {
    case TokenKind::StringLiteral: {
        auto iter = m_stringLiterals.find(lexeme);
        if (iter != m_stringLiterals.end()) {
            constant = iter->second;
        } else {
            constant = llvm::ConstantDataArray::getString(
                m_context,
                view_to_stringRef(lexeme),
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

            llvm::Constant* zero_32 = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(m_context));
            std::array indices{
                zero_32,
                zero_32
            };

            constant = llvm::ConstantExpr::getGetElementPtr(nullptr, value, indices, true);
            m_stringLiterals.emplace(lexeme, constant);
        }
        break;
    }
    case TokenKind::NumberLiteral: {
        uint64_t result = 0;
        std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), result); // NOLINT
        constant = llvm::ConstantInt::get(
            ast->type->llvmType(m_context),
            result,
            llvm::cast<TypeNumber>(ast->type)->isSigned());
        break;
    }
    case TokenKind::BooleanLiteral: {
        uint64_t value = lexeme == "TRUE" ? 1 : 0;
        constant = llvm::ConstantInt::get(
            ast->type->llvmType(m_context),
            value,
            llvm::cast<TypeNumber>(ast->type)->isSigned());
        break;
    }
    case TokenKind::NullLiteral:
        constant = llvm::ConstantPointerNull::get(
            llvm::cast<llvm::PointerType>(ast->type->llvmType(m_context)));
        break;
    default:
        error("Invalid literal type");
    }

    ast->llvmValue = constant;
}

unique_ptr<llvm::Module> CodeGen::getModule() {
    return std::move(m_module);
}
