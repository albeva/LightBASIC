//
// Created by Albert Varaksin on 05/07/2020.
//
#include "CodeGen.hpp"
#include "Builders/BinaryExprBuilder.hpp"
#include "Builders/DoLoopBuilder.hpp"
#include "Builders/ForStmtBuilder.hpp"
#include "Builders/IfStmtBuilder.hpp"
#include "Driver/Context.hpp"
#include "Helpers.hpp"
#include "Type/Type.hpp"
#include "ValueHandler.hpp"
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>
using namespace lbc;

CodeGen::CodeGen(Context& context) noexcept
: m_context{ context },
  m_llvmContext{ context.getLlvmContext() },
  m_builder{ m_llvmContext } {
    m_constantTrue = llvm::cast<llvm::ConstantInt>(llvm::ConstantInt::get( // NOLINT
        TypeBoolean::get()->getLlvmType(m_context),
        1,
        false));
    m_constantFalse = llvm::cast<llvm::ConstantInt>(llvm::ConstantInt::get( // NOLINT
        TypeBoolean::get()->getLlvmType(m_context),
        0,
        false));
}

bool CodeGen::validate() const noexcept {
    assert(m_module != nullptr); // NOLINT
    return !llvm::verifyModule(*m_module, &llvm::outs());
}

void CodeGen::addBlock() noexcept {
    auto* func = m_builder.GetInsertBlock()->getParent();
    auto* block = llvm::BasicBlock::Create(m_llvmContext, "", func);
    m_builder.SetInsertPoint(block);
}

void CodeGen::terminateBlock(llvm::BasicBlock* dest) noexcept {
    if (m_builder.GetInsertBlock()->getTerminator() == nullptr) {
        m_builder.CreateBr(dest);
    }
}

void CodeGen::switchBlock(llvm::BasicBlock* block) noexcept {
    assert(block != nullptr);

    terminateBlock(block);
    if (block->getParent() != nullptr) {
        block->moveAfter(m_builder.GetInsertBlock());
    } else {
        block->insertInto(m_builder.GetInsertBlock()->getParent());
    }
    m_builder.SetInsertPoint(block);
}

void CodeGen::visit(AstModule& ast) noexcept {
    m_astRootModule = &ast;
    m_fileId = ast.fileId;
    m_scope = Scope::Root;
    auto file = m_context.getSourceMrg().getMemoryBuffer(m_fileId)->getBufferIdentifier();

    m_module = make_unique<llvm::Module>(file, m_llvmContext);
    m_module->setTargetTriple(m_context.getTriple().str());

    declareFuncs();

    if (m_context.getTriple().isOSWindows()) {
        auto* chkstk = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(m_llvmContext), false),
            llvm::Function::InternalLinkage,
            "__chkstk",
            *m_module);
        chkstk->setCallingConv(llvm::CallingConv::C);
        chkstk->setDSOLocal(true);
        chkstk->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Local);
        auto* block = llvm::BasicBlock::Create(m_llvmContext, "entry", chkstk);
        m_builder.SetInsertPoint(block);
        m_builder.CreateRetVoid();
    }

    bool hasMainDefined = false;
    if (auto* main = ast.symbolTable->find("MAIN")) {
        if (main->alias() == "main") {
            hasMainDefined = true;
        }
    }
    bool generateMain = !hasMainDefined && ast.hasImplicitMain;

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
        auto* block = llvm::BasicBlock::Create(m_llvmContext, "entry", mainFn);
        m_builder.SetInsertPoint(block);
    } else {
        auto* block = llvm::BasicBlock::Create(m_llvmContext, "entry");
        m_builder.SetInsertPoint(block);
    }

    // parse statements
    visit(*ast.stmtList);

    // close main
    if (mainFn != nullptr) {
        auto* retValue = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(m_llvmContext));
        auto& lastBlock = mainFn->getBasicBlockList().back();
        llvm::ReturnInst::Create(m_llvmContext, retValue, &lastBlock);
    }

    if (m_globalCtorFunc != nullptr) {
        auto* block = getGlobalCtorBlock();
        if (block->getTerminator() == nullptr) {
            llvm::ReturnInst::Create(m_llvmContext, nullptr, block);
        }
    }
}

llvm::BasicBlock* CodeGen::getGlobalCtorBlock() noexcept {
    if (m_globalCtorFunc == nullptr) {
        m_globalCtorFunc = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getVoidTy(m_llvmContext), false),
            llvm::Function::InternalLinkage,
            "__lbc_global_var_init",
            *m_module);
        if (m_context.getTriple().isOSBinFormatMachO()) {
            m_globalCtorFunc->setSection("__TEXT,__StaticInit,regular,pure_instructions");
        } else if (m_context.getTriple().isOSBinFormatELF()) {
            m_globalCtorFunc->setSection(".text.startup");
        }
        llvm::appendToGlobalCtors(*m_module, m_globalCtorFunc, 0, nullptr);
        llvm::BasicBlock::Create(m_llvmContext, "entry", m_globalCtorFunc);
    }
    return &m_globalCtorFunc->getBasicBlockList().back();
}

void CodeGen::visit(AstStmtList& ast) noexcept {
    for (const auto& stmt : ast.stmts) {
        visit(*stmt);
    }
}

void CodeGen::visit(AstAssignStmt& ast) noexcept {
    auto* lvalue = getStoreValue(*ast.lhs);
    auto* rvalue = visit(*ast.rhs);
    m_builder.CreateStore(rvalue, lvalue);
}

llvm::Value* CodeGen::getStoreValue(AstExpr& ast) noexcept {
    if (auto* id = dyn_cast<AstIdentExpr>(&ast)) {
        return id->symbol->getLlvmValue();
    }
    fatalError("Unsupported assign target");
}

void CodeGen::visit(AstExprStmt& ast) noexcept {
    visit(*ast.expr);
}

// Variables

void CodeGen::visit(AstVarDecl& ast) noexcept {
    if (m_declareAsGlobals) {
        declareGlobalVar(ast);
    } else {
        declareLocalVar(ast);
    }
}

void CodeGen::declareGlobalVar(AstVarDecl& ast) noexcept {
    auto* sym = ast.symbol;
    llvm::Constant* constant = nullptr;
    llvm::Type* exprType = sym->type()->getLlvmType(m_context);
    bool generateStoreInCtror = false;

    // has an init expr?
    if (ast.expr) {
        if (auto* litExpr = dyn_cast<AstLiteralExpr>(ast.expr.get())) {
            auto* rvalue = visit(*litExpr);
            constant = llvm::cast<llvm::Constant>(rvalue);
        } else {
            generateStoreInCtror = true;
        }
    }

    if (constant == nullptr) {
        constant = llvm::Constant::getNullValue(exprType);
    }
    llvm::Value* lvalue = new llvm::GlobalVariable(
        *m_module,
        exprType,
        false,
        sym->getLlvmLinkage(),
        constant,
        ast.symbol->identifier());

    if (generateStoreInCtror) {
        auto* block = m_builder.GetInsertBlock();
        m_builder.SetInsertPoint(getGlobalCtorBlock());
        auto* rvalue = visit(*ast.expr);
        m_builder.CreateStore(rvalue, lvalue);
        m_builder.SetInsertPoint(block);
    }

    sym->setLlvmValue(lvalue);
}

void CodeGen::declareLocalVar(AstVarDecl& ast) noexcept {
    llvm::Value* rvalue = nullptr;
    llvm::Type* exprType = ast.symbol->type()->getLlvmType(m_context);

    // has an init expr?
    if (ast.expr) {
        rvalue = visit(*ast.expr);
    }

    auto* lvalue = m_builder.CreateAlloca(exprType, nullptr, ast.symbol->identifier());

    if (rvalue != nullptr) {
        m_builder.CreateStore(rvalue, lvalue);
    }

    ast.symbol->setLlvmValue(lvalue);
}

// Functions

void CodeGen::visit(AstFuncDecl& /*ast*/) noexcept {
    // NOOP
}

void CodeGen::declareFuncs() noexcept {
    for (const auto& stmt : m_astRootModule->stmtList->stmts) {
        switch (stmt->kind) {
        case AstKind::FuncDecl:
            declareFunc(static_cast<AstFuncDecl&>(*stmt));
            break;
        case AstKind::FuncStmt:
            declareFunc(*static_cast<AstFuncStmt&>(*stmt).decl);
            break;
        default:
            break;
        }
    }
}

void CodeGen::declareFunc(AstFuncDecl& ast) noexcept {
    auto* sym = ast.symbol;
    auto* fnTy = llvm::cast<llvm::FunctionType>(ast.symbol->type()->getLlvmType(m_context));
    auto* fn = llvm::Function::Create(
        fnTy,
        sym->getLlvmLinkage(),
        ast.symbol->identifier(),
        *m_module);
    fn->setCallingConv(llvm::CallingConv::C);
    fn->setDSOLocal(true);
    ast.symbol->setLlvmValue(fn);

    auto* iter = fn->arg_begin();
    for (const auto& param : ast.paramDecls) {
        iter->setName(param->symbol->identifier());
        param->symbol->setLlvmValue(iter);
        iter++; // NOLINT
    }
}

void CodeGen::visit(AstFuncParamDecl& /*ast*/) noexcept {
    llvm_unreachable("visitFuncParamDecl");
}

void CodeGen::visit(AstFuncStmt& ast) noexcept {
    RESTORE_ON_EXIT(m_scope);
    m_scope = Scope::Function;

    RESTORE_ON_EXIT(m_declareAsGlobals);
    m_declareAsGlobals = false;

    auto* func = llvm::cast<llvm::Function>(ast.decl->symbol->getLlvmValue());

    auto* current = m_builder.GetInsertBlock();
    auto* block = llvm::BasicBlock::Create(m_llvmContext, "", func);
    m_builder.SetInsertPoint(block);

    for (const auto& param : ast.decl->paramDecls) {
        auto* sym = param->symbol;
        auto* value = sym->getLlvmValue();
        sym->setLlvmValue(m_builder.CreateAlloca(
            sym->type()->getLlvmType(m_context),
            nullptr,
            sym->identifier() + ".addr"));
        m_builder.CreateStore(value, sym->getLlvmValue());
    }

    visit(*ast.stmtList);

    block = m_builder.GetInsertBlock();
    if (block->getTerminator() == nullptr) {
        auto* retType = func->getReturnType();
        if (!retType->isVoidTy()) {
            fatalError("No RETURN statement");
        }
        m_builder.CreateRetVoid();
    }

    m_builder.SetInsertPoint(current);
}

void CodeGen::visit(AstReturnStmt& ast) noexcept {
    if (ast.expr) {
        auto* value = visit(*ast.expr);
        m_builder.CreateRet(value);
    } else {
        auto* func = m_builder.GetInsertBlock()->getParent();
        auto* retTy = func->getReturnType();

        if (retTy->isVoidTy()) {
            m_builder.CreateRetVoid();
        } else {
            auto* constant = llvm::Constant::getNullValue(retTy);
            m_builder.CreateRet(constant);
        }
    }
}

//------------------------------------------------------------------
// Type (user defined)
//------------------------------------------------------------------

void CodeGen::visit(AstTypeDecl& /*ast*/) noexcept {
    llvm_unreachable("CodeGen::AstTypeDecl");
}

//------------------------------------------------------------------
// IF stmt
//------------------------------------------------------------------

void CodeGen::visit(AstIfStmt& ast) noexcept {
    RESTORE_ON_EXIT(m_declareAsGlobals);
    m_declareAsGlobals = false;
    Gen::IfStmtBuilder(*this, ast);
}

void CodeGen::visit(AstForStmt& ast) noexcept {
    RESTORE_ON_EXIT(m_declareAsGlobals);
    m_declareAsGlobals = false;
    Gen::ForStmtBuilder(*this, ast);
}

void CodeGen::visit(AstDoLoopStmt& ast) noexcept {
    RESTORE_ON_EXIT(m_declareAsGlobals);
    m_declareAsGlobals = false;
    Gen::DoLoopBuilder(*this, ast);
}

void CodeGen::visit(AstControlFlowBranch& ast) noexcept {
    auto target = m_controlStack.find(ast.destination);
    if (target == m_controlStack.cend()) {
        fatalError("control statement not found");
    }

    switch (ast.action) {
    case AstControlFlowBranch::Action::Continue:
        m_builder.CreateBr(target->second.continueBlock);
        break;
    case AstControlFlowBranch::Action::Exit:
        m_builder.CreateBr(target->second.exitBlock);
        break;
    }

    addBlock();
}

//------------------------------------------------------------------
// Attributes
//------------------------------------------------------------------

void CodeGen::visit(AstAttributeList& /*ast*/) noexcept {
    llvm_unreachable("visitAttributeList");
}

void CodeGen::visit(AstAttribute& /*ast*/) noexcept {
    llvm_unreachable("visitAttribute");
}

llvm::Value* CodeGen::visit(AstIdentExpr& ast) noexcept {
    const auto* type = ast.type;
    if (type->isFunction()) {
        return ast.symbol->getLlvmValue();
    }

    auto* sym = ast.symbol;
    return m_builder.CreateLoad(sym->getLlvmValue(), sym->identifier());
}

void CodeGen::visit(AstTypeExpr& /*ast*/) noexcept {
    // NOOP
}

//------------------------------------------------------------------
// Expressions
//------------------------------------------------------------------

llvm::Value* CodeGen::visit(AstCallExpr& ast) noexcept {
    auto* fn = llvm::cast<llvm::Function>(visit(*ast.identExpr));

    std::vector<llvm::Value*> args;
    args.reserve(ast.argExprs.size());
    for (const auto& arg : ast.argExprs) {
        auto* value = visit(*arg);
        args.emplace_back(value);
    }

    auto* call = m_builder.CreateCall(llvm::FunctionCallee(fn), args, "");
    call->setTailCall(false);
    return call;
}

llvm::Value* CodeGen::visit(AstLiteralExpr& ast) noexcept {
    auto visitor = Visitor{
        [&](std::monostate /*value*/) -> llvm::Value* {
            return llvm::ConstantPointerNull::get(
                llvm::cast<llvm::PointerType>(ast.type->getLlvmType(m_context)));
        },
        [&](StringRef str) -> llvm::Value* {
            return getStringConstant(str);
        },
        [&](uint64_t value) -> llvm::Value* {
            return llvm::ConstantInt::get(
                ast.type->getLlvmType(m_context),
                value,
                static_cast<const TypeIntegral*>(ast.type)->isSigned());
        },
        [&](double value) -> llvm::Value* {
            return llvm::ConstantFP::get(
                ast.type->getLlvmType(m_context),
                value);
        },
        [&](bool value) -> llvm::Value* {
            return value ? m_constantTrue : m_constantFalse;
        }
    };
    return std::visit(visitor, ast.value);
}

llvm::Value* CodeGen::getStringConstant(StringRef str) noexcept {
    auto [iter, inserted] = m_stringLiterals.try_emplace(str, nullptr);
    if (!inserted) {
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
    iter->second = constant;

    return constant;
}

llvm::Value* CodeGen::visit(AstUnaryExpr& ast) noexcept {
    switch (ast.tokenKind) {
    case TokenKind::Negate: {
        auto* value = visit(*ast.expr);

        if (value->getType()->isIntegerTy()) {
            return m_builder.CreateNeg(value);
        }

        if (value->getType()->isFloatingPointTy()) {
            return m_builder.CreateFNeg(value);
        }

        llvm_unreachable("Unexpected unary operator");
    }
    case TokenKind::LogicalNot: {
        auto* value = visit(*ast.expr);
        return m_builder.CreateNot(value, "lnot");
    }
    default:
        llvm_unreachable("Unexpected unary operator");
    }
}

llvm::Value* CodeGen::visit(AstDereference& ast) noexcept {
    auto* value = visit(*ast.expr);
    return m_builder.CreateLoad(value);
}

llvm::Value* CodeGen::visit(AstAddressOf& ast) noexcept {
    if (auto* ident = dyn_cast<AstIdentExpr>(ast.expr.get())) {
        return ident->symbol->getLlvmValue();
    }
    fatalError("Taking address of non id expression");
}

//------------------------------------------------------------------
// Binary Operation
//------------------------------------------------------------------

llvm::Value* CodeGen::visit(AstBinaryExpr& ast) noexcept {
    return Gen::BinaryExprBuilder(*this, ast).build();
}

//------------------------------------------------------------------
// Casting
//------------------------------------------------------------------

llvm::Value* CodeGen::visit(AstCastExpr& ast) noexcept {
    auto* value = visit(*ast.expr);

    bool srcIsSigned = ast.expr->type->isSignedIntegral();
    bool dstIsSigned = ast.type->isSignedIntegral();

    auto opcode = llvm::CastInst::getCastOpcode(
        value,
        srcIsSigned,
        ast.type->getLlvmType(m_context),
        dstIsSigned);
    return m_builder.CreateCast(opcode, value, ast.type->getLlvmType(m_context));
}

llvm::Value* CodeGen::visit(AstIfExpr& ast) noexcept {
    auto* condValue = visit(*ast.expr);
    auto* trueValue = visit(*ast.trueExpr);
    auto* falseValue = visit(*ast.falseExpr);

    return m_builder.CreateSelect(condValue, trueValue, falseValue);
}

unique_ptr<llvm::Module> CodeGen::getModule() noexcept {
    return std::move(m_module);
}
