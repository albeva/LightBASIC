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
#include <llvm/Transforms/Utils/ModuleUtils.h>
using namespace lbc;

CodeGen::CodeGen(Context& context) noexcept
: m_context{ context },
  m_llvmContext{ context.getLlvmContext() },
  m_builder{ m_llvmContext } {
}

bool CodeGen::validate() const noexcept {
    assert(m_module != nullptr); // NOLINT
    return !llvm::verifyModule(*m_module, &llvm::outs());
}

void CodeGen::print() const noexcept {
    assert(m_module != nullptr); // NOLINT
    auto* printer = llvm::createPrintModulePass(llvm::outs());
    printer->runOnModule(*m_module);
}

void CodeGen::visit(AstModule* ast) noexcept {
    m_astRootModule = ast;
    m_fileId = ast->fileId;
    m_scope = Scope::Root;
    auto file = m_context.getSourceMrg().getMemoryBuffer(m_fileId)->getBufferIdentifier();

    m_module = make_unique<llvm::Module>(file, m_llvmContext);
    m_module->setTargetTriple(m_context.getTriple().str());

    declareFuncs();

    bool hasMainDefined = false;
    if (auto* main = ast->symbolTable->find("MAIN")) {
        if (main->alias() == "main") {
            hasMainDefined = true;
        }
    }
    bool generateMain = !hasMainDefined && ast->hasImplicitMain;

    llvm::Function* mainFn = nullptr;
    if (generateMain) {
        mainFn = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(m_llvmContext), false),
            llvm::Function::ExternalLinkage,
            "main",
            *m_module);
        mainFn->setCallingConv(llvm::CallingConv::C);
        mainFn->setDSOLocal(true);
        mainFn->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Local);
        auto* block = llvm::BasicBlock::Create(m_llvmContext, "", mainFn);
        m_builder.SetInsertPoint(block);
    } else {
        auto* block = llvm::BasicBlock::Create(m_llvmContext);
        m_builder.SetInsertPoint(block);
    }

    // parse statements
    visit(ast->stmtList.get());

    // close main
    if (generateMain) {
        auto* retValue = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(m_llvmContext));
        auto& lastBlock = mainFn->getBasicBlockList().back();
        llvm::ReturnInst::Create(m_llvmContext, retValue, &lastBlock);
    }

    if (m_globalCtorBlock != nullptr) {
        if (m_globalCtorBlock->getTerminator() == nullptr) {
            llvm::ReturnInst::Create(m_llvmContext, nullptr, m_globalCtorBlock);
        }
    }
}

llvm::BasicBlock* CodeGen::getGlobalCtorBlock() noexcept {
    if (m_globalCtorBlock == nullptr) {
        auto* ctorFn = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(m_llvmContext), false),
            llvm::Function::InternalLinkage,
            "__lbc_global_var_init",
            *m_module);
        if (m_context.getTriple().isOSBinFormatMachO()) {
            ctorFn->setSection("__TEXT,__StaticInit,regular,pure_instructions");
        } else if (m_context.getTriple().isOSBinFormatELF()) {
            ctorFn->setSection(".text.startup");
        }
        llvm::appendToGlobalCtors(*m_module, ctorFn, 0, nullptr);
        m_globalCtorBlock = llvm::BasicBlock::Create(m_llvmContext, "", ctorFn);
    }
    return m_globalCtorBlock;
}

void CodeGen::visit(AstStmtList* ast) noexcept {
    for (const auto& stmt : ast->stmts) {
        visit(stmt.get());
    }
}

void CodeGen::visit(AstAssignStmt* ast) noexcept {
    auto* dstValue = getStoreValue(ast->identExpr.get());
    visit(ast->expr.get());

    m_builder.CreateStore(ast->expr->llvmValue, dstValue);
}

llvm::Value* CodeGen::getStoreValue(AstIdentExpr* identExpr) {
    return identExpr->symbol->getLlvmValue();
}

void CodeGen::visit(AstExprStmt* ast) noexcept {
    visit(ast->expr.get());
}

// Variables

void CodeGen::visit(AstVarDecl* ast) noexcept {
    if (m_scope == Scope::Root) {
        declareGlobalVar(ast);
    } else {
        declareLocalVar(ast);
    }
}

void CodeGen::declareGlobalVar(AstVarDecl* ast) noexcept {
    auto* sym = ast->symbol;
    llvm::Constant* constant = nullptr;
    llvm::Type* exprType = sym->type()->getLlvmType(m_context);
    bool generateStoreInCtror = false;

    // has an init expr?
    if (ast->expr) {
        if (auto* litExpr = dyn_cast<AstLiteralExpr>(ast->expr.get())) {
            visit(litExpr);
            constant = llvm::cast<llvm::Constant>(ast->expr->llvmValue);
        } else {
            generateStoreInCtror = true;
        }
    }

    if (constant == nullptr) {
        constant = llvm::Constant::getNullValue(exprType);
    }
    llvm::Value* value = new llvm::GlobalVariable(
        *m_module,
        exprType,
        false,
        sym->getLlvmLinkage(),
        constant,
        ast->symbol->identifier());

    if (generateStoreInCtror) {
        auto* block = m_builder.GetInsertBlock();
        m_builder.SetInsertPoint(getGlobalCtorBlock());
        visit(ast->expr.get());
        m_builder.CreateStore(ast->expr->llvmValue, value);
        m_builder.SetInsertPoint(block);
    }

    sym->setLlvmValue(value);
}

void CodeGen::declareLocalVar(AstVarDecl* ast) noexcept {
    llvm::Value* exprValue = nullptr;
    llvm::Type* exprType = ast->symbol->type()->getLlvmType(m_context);

    // has an init expr?
    if (ast->expr) {
        visit(ast->expr.get());
        exprValue = ast->expr->llvmValue;
    }

    auto* value = m_builder.CreateAlloca(exprType, nullptr, ast->symbol->identifier());

    if (exprValue != nullptr) {
        // new llvm::StoreInst(exprValue, value, m_block);
        m_builder.CreateStore(exprValue, value);
    }

    ast->symbol->setLlvmValue(value);
}

// Functions

void CodeGen::visit(AstFuncDecl* /*ast*/) noexcept {
    // NOOP
}

void CodeGen::declareFuncs() noexcept {
    for (const auto& stmt : m_astRootModule->stmtList->stmts) {
        switch (stmt->kind()) {
        case AstKind::FuncDecl:
            declareFunc(static_cast<AstFuncDecl*>(stmt.get())); // NOLINT
            break;
        case AstKind::FuncStmt:
            declareFunc(static_cast<AstFuncStmt*>(stmt.get())->decl.get()); // NOLINT
            break;
        default:
            break;
        }
    }
}

void CodeGen::declareFunc(AstFuncDecl* ast) noexcept {
    auto* sym = ast->symbol;
    auto* fnTy = llvm::cast<llvm::FunctionType>(ast->symbol->type()->getLlvmType(m_context));
    auto* fn = llvm::Function::Create(
        fnTy,
        sym->getLlvmLinkage(),
        ast->symbol->identifier(),
        *m_module);
    fn->setCallingConv(llvm::CallingConv::C);
    fn->setDSOLocal(true);
    ast->symbol->setLlvmValue(fn);

    auto* iter = fn->arg_begin();
    for (const auto& param : ast->paramDecls) {
        iter->setName(param->symbol->identifier());
        param->symbol->setLlvmValue(iter);
        iter++; // NOLINT
    }
}

void CodeGen::visit(AstFuncParamDecl* /*ast*/) noexcept {
    llvm_unreachable("visitFuncParamDecl");
}

void CodeGen::visit(AstFuncStmt* ast) noexcept {
    RESTORE_ON_EXIT(m_scope);
    m_scope = Scope::Function;
    auto* func = llvm::cast<llvm::Function>(ast->decl->symbol->getLlvmValue());

    auto* current = m_builder.GetInsertBlock();
    auto* block = llvm::BasicBlock::Create(m_llvmContext, "", func);
    m_builder.SetInsertPoint(block);

    for (const auto& param : ast->decl->paramDecls) {
        auto* sym = param->symbol;
        auto* value = sym->getLlvmValue();
        sym->setLlvmValue(m_builder.CreateAlloca(
            sym->type()->getLlvmType(m_context),
            nullptr,
            sym->identifier() + ".addr"));
        m_builder.CreateStore(value, sym->getLlvmValue());
    }

    visit(ast->stmtList.get());

    if (block->getTerminator() == nullptr) {
        auto* retType = func->getReturnType();
        if (!retType->isVoidTy()) {
            fatalError("No RETURN statement");
        }
        m_builder.CreateRetVoid();
    }

    m_builder.SetInsertPoint(current);
}

void CodeGen::visit(AstReturnStmt* ast) noexcept {
    if (ast->expr) {
        visit(ast->expr.get());
        m_builder.CreateRet(ast->expr->llvmValue);
    } else {
        m_builder.CreateRetVoid();
    }
}

void CodeGen::visit(AstAttributeList* /*ast*/) noexcept {
    llvm_unreachable("visitAttributeList");
}

void CodeGen::visit(AstAttribute* /*ast*/) noexcept {
    llvm_unreachable("visitAttribute");
}

void CodeGen::visit(AstIdentExpr* ast) noexcept {
    const auto* type = ast->type;
    if (type->isFunction()) {
        ast->llvmValue = ast->symbol->getLlvmValue();
        return;
    }

    auto* sym = ast->symbol;
    ast->llvmValue = m_builder.CreateLoad(sym->getLlvmValue(), sym->identifier());
}

void CodeGen::visit(AstTypeExpr* /*ast*/) noexcept {
    // NOOP
}

void CodeGen::visit(AstCallExpr* ast) noexcept {
    visit(ast->identExpr.get());

    auto* fn = llvm::cast<llvm::Function>(ast->identExpr->llvmValue);

    std::vector<llvm::Value*> args;
    args.reserve(ast->argExprs.size());
    for (const auto& arg : ast->argExprs) {
        visit(arg.get());
        args.emplace_back(arg->llvmValue);
    }

    auto* call = m_builder.CreateCall(llvm::FunctionCallee(fn), args, "");
    call->setTailCall(false);
    ast->llvmValue = call;
}

void CodeGen::visit(AstLiteralExpr* ast) noexcept {
    llvm::Constant* constant = nullptr;
    auto visitor = Visitor{
        [&](std::monostate /*value*/) {
            constant = llvm::ConstantPointerNull::get(
                llvm::cast<llvm::PointerType>(ast->type->getLlvmType(m_context)));
        },
        [&](const StringRef& str) {
            constant = getStringConstant(str);
        },
        [&](uint64_t value) {
            constant = llvm::ConstantInt::get(
                ast->type->getLlvmType(m_context),
                value,
                static_cast<const TypeIntegral*>(ast->type)->isSigned()); // NOLINT
        },
        [&](double value) {
            constant = llvm::ConstantFP::get(
                ast->type->getLlvmType(m_context),
                value);
        },
        [&](bool value) {
            constant = llvm::ConstantInt::get(
                ast->type->getLlvmType(m_context),
                value ? 1 : 0,
                false);
        }
    };
    std::visit(visitor, ast->value);
    ast->llvmValue = constant;
}

llvm::Constant* CodeGen::getStringConstant(const StringRef& str) noexcept {
    auto iter = m_stringLiterals.find(str);
    if (iter != m_stringLiterals.end()) {
        return iter->second;
    }

    auto* constant = llvm::ConstantDataArray::getString(
        m_llvmContext,
        str,
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
    m_stringLiterals.insert({ str, constant });

    return constant;
}

void CodeGen::visit(AstUnaryExpr* ast) noexcept {
    switch (ast->tokenKind) {
    case TokenKind::Negate: {
        auto* expr = ast->expr.get();
        visit(expr);

        if (expr->llvmValue->getType()->isIntegerTy()) {
            ast->llvmValue = m_builder.CreateNeg(expr->llvmValue);
        } else if (expr->llvmValue->getType()->isFloatingPointTy()) {
            ast->llvmValue = m_builder.CreateFNeg(expr->llvmValue);
        }

        break;
    }
    case TokenKind::LogicalNot: {
        auto* expr = ast->expr.get();
        visit(expr);
        ast->llvmValue = m_builder.CreateNot(expr->llvmValue, "lnot");
        break;
    }
    default:
        llvm_unreachable("Unexpected unary operator");
    }
}

void CodeGen::visit(AstBinaryExpr* ast) noexcept {
    // TODO
}

void CodeGen::visit(AstCastExpr* ast) noexcept {
    visit(ast->expr.get());

    bool srcIsSigned = ast->expr->type->isSignedIntegral();
    bool dstIsSigned = ast->type->isSignedIntegral();

    auto opcode = llvm::CastInst::getCastOpcode(
        ast->expr->llvmValue,
        srcIsSigned,
        ast->type->getLlvmType(m_context),
        dstIsSigned);
    ast->llvmValue = m_builder.CreateCast(opcode, ast->expr->llvmValue, ast->type->getLlvmType(m_context));
}

unique_ptr<llvm::Module> CodeGen::getModule() noexcept {
    return std::move(m_module);
}
