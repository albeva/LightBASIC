//
// Created by Albert Varaksin on 05/07/2020.
//
#include "CodeGen.h"
#include "Driver/Context.h"
#include "Type/Type.h"
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
    return true; // !llvm::verifyModule(*m_module, &llvm::outs());
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
        auto* block = llvm::BasicBlock::Create(m_llvmContext, "entry", mainFn);
        m_builder.SetInsertPoint(block);
    } else {
        auto* block = llvm::BasicBlock::Create(m_llvmContext, "entry");
        m_builder.SetInsertPoint(block);
    }

    // parse statements
    visit(ast->stmtList.get());

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
    if (m_declareAsGlobals) {
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

    RESTORE_ON_EXIT(m_declareAsGlobals);
    m_declareAsGlobals = false;

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

void CodeGen::visit(AstReturnStmt* ast) noexcept {
    if (ast->expr) {
        visit(ast->expr.get());
        m_builder.CreateRet(ast->expr->llvmValue);
    } else {
        m_builder.CreateRetVoid();
    }
}

//------------------------------------------------------------------
// IF stmt
//------------------------------------------------------------------

void CodeGen::visit(AstIfStmt* ast) noexcept {
    RESTORE_ON_EXIT(m_declareAsGlobals);
    m_declareAsGlobals = false;

    auto* func = m_builder.GetInsertBlock()->getParent();
    auto* endBlock = llvm::BasicBlock::Create(m_llvmContext, "if.end", func);

    const auto count = ast->blocks.size();
    for (size_t idx = 0; idx < count; idx++) {
        const auto& block = ast->blocks[idx];
        llvm::BasicBlock* elseBlock = nullptr;

        for (const auto& decl: block.decls) {
            visit(decl.get());
        }

        if (block.expr) {
            visit(block.expr.get());

            auto* thenBlock = llvm::BasicBlock::Create(m_llvmContext, "if.then", func);
            if (idx == count - 1) {
                elseBlock = endBlock;
            } else {
                elseBlock = llvm::BasicBlock::Create(m_llvmContext, "if.else", func);
            }
            m_builder.CreateCondBr(block.expr->llvmValue, thenBlock, elseBlock);

            thenBlock->moveAfter(m_builder.GetInsertBlock());
            m_builder.SetInsertPoint(thenBlock);
        } else {
            elseBlock = endBlock;
        }

        visit(block.stmt.get());
        m_builder.CreateBr(endBlock);

        elseBlock->moveAfter(m_builder.GetInsertBlock());
        m_builder.SetInsertPoint(elseBlock);
    }
}

void CodeGen::visit(AstForStmt* ast) noexcept {
    RESTORE_ON_EXIT(m_declareAsGlobals);
    m_declareAsGlobals = false;

    // blocks
    auto* func = m_builder.GetInsertBlock()->getParent();
    auto* exitBlock = llvm::BasicBlock::Create(m_llvmContext, "for.end");
    auto* bodyBlock = llvm::BasicBlock::Create(m_llvmContext, "for.body");
    auto* condBlock = llvm::BasicBlock::Create(m_llvmContext, "for.cond");

    for (const auto& decl: ast->decls) {
        visit(decl.get());
    }
    visit(ast->iterator.get());
    auto* iterator = ast->iterator->symbol->getLlvmValue();
    const auto* type = ast->iterator->symbol->type();
    auto* llvmType = type->getLlvmType(m_context);

    bool isIntegral = type->isIntegral();
    auto isSigned = type->isSignedIntegral();
    llvm::CmpInst::Predicate lessThanPred{};
    llvm::CmpInst::Predicate lessOrEqualPred{};
    if (isIntegral) {
        if (isSigned) {
            lessThanPred = llvm::CmpInst::ICMP_SLT;
            lessOrEqualPred = llvm::CmpInst::ICMP_SLE;
        } else {
            lessThanPred = llvm::CmpInst::ICMP_ULT;
            lessOrEqualPred = llvm::CmpInst::ICMP_ULE;
        }
    } else {
        lessThanPred = llvm::CmpInst::FCMP_OLT;
        lessOrEqualPred = llvm::CmpInst::FCMP_OLE;
    }

    visit(ast->limit.get());
    auto* limit = m_builder.CreateAlloca(llvmType, nullptr, "__for.limit");
    m_builder.CreateStore(ast->limit->llvmValue, limit);

    auto* limitValue = m_builder.CreateLoad(limit);
    auto* iterValue = m_builder.CreateLoad(iterator);
    auto* isDecr = m_builder.CreateCmp(
        lessThanPred,
        limitValue,
        iterValue);

    auto * step = m_builder.CreateAlloca(llvmType, nullptr, "__for.step");
    if (ast->step) {
        visit(ast->step.get());
        m_builder.CreateStore(ast->step->llvmValue, step);

        llvm::Value* stepValue = m_builder.CreateLoad(step);
        auto* isStepNeg = m_builder.CreateCmp(
            lessThanPred,
            stepValue,
            llvm::Constant::getNullValue(llvmType));

        auto* negateBlock = llvm::BasicBlock::Create(m_llvmContext, "for.negate");
        auto* negateEndBlock = llvm::BasicBlock::Create(m_llvmContext, "for.negate.end");
        m_builder.CreateCondBr(isStepNeg, negateBlock, negateEndBlock);

        negateBlock->insertInto(func);
        m_builder.SetInsertPoint(negateBlock);
        stepValue = m_builder.CreateNeg(stepValue);
        m_builder.CreateStore(stepValue, step);
        m_builder.CreateBr(negateEndBlock);

        negateEndBlock->insertInto(func);
        m_builder.SetInsertPoint(negateEndBlock);

        auto* isDecrBlock = llvm::BasicBlock::Create(m_llvmContext, "for.isdecr");
        auto* isIncrBlock = llvm::BasicBlock::Create(m_llvmContext, "for.isincr");
        m_builder.CreateCondBr(isDecr, isDecrBlock, isIncrBlock);

        isDecrBlock->insertInto(func);
        m_builder.SetInsertPoint(isDecrBlock);
        m_builder.CreateCondBr(isStepNeg, bodyBlock, exitBlock);

        isIncrBlock->insertInto(func);
        m_builder.SetInsertPoint(isIncrBlock);
        m_builder.CreateCondBr(isStepNeg, exitBlock, bodyBlock);
    } else {
        llvm::Value* stepVal = nullptr;
        if (isIntegral) {
            stepVal = llvm::ConstantInt::get(llvmType, 1, isSigned);
        } else {
            stepVal = llvm::ConstantFP::get(llvmType, 1);
        }
        m_builder.CreateStore(stepVal, step);
    }

    if (m_builder.GetInsertBlock()->getTerminator() == nullptr) {
        m_builder.CreateBr(bodyBlock);
    }

    bodyBlock->insertInto(func);
    m_builder.SetInsertPoint(bodyBlock);
    visit(ast->stmt.get());
    m_builder.CreateBr(condBlock);

    condBlock->insertInto(func);
    m_builder.SetInsertPoint(condBlock);
    auto* incrBlock = llvm::BasicBlock::Create(m_llvmContext, "for.cond.incr");
    auto* decrBlock = llvm::BasicBlock::Create(m_llvmContext, "for.cond.decr");
    m_builder.CreateCondBr(isDecr, decrBlock, incrBlock);

    const auto conditional = [&](llvm::BasicBlock* block, bool incr) {
        block->insertInto(func);
        m_builder.SetInsertPoint(block);

        llvm::Value* iterValue = m_builder.CreateLoad(iterator);
        llvm::Value* stepValue = m_builder.CreateLoad(step);
        auto* result = incr
            ? m_builder.CreateAdd(iterValue, stepValue)
            : m_builder.CreateSub(iterValue, stepValue);
        m_builder.CreateStore(result, iterator);

        auto* limitValue = m_builder.CreateLoad(limit);
        auto* cmp = incr
            ? m_builder.CreateCmp(lessOrEqualPred, result, limitValue)
            : m_builder.CreateCmp(lessOrEqualPred, limitValue, result);
        m_builder.CreateCondBr(cmp, bodyBlock, exitBlock);
    };

    conditional(incrBlock, true);
    conditional(decrBlock, false);

    exitBlock->insertInto(func);
    m_builder.SetInsertPoint(exitBlock);
}

//------------------------------------------------------------------
// Attributes
//------------------------------------------------------------------

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
        [&](StringRef str) {
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
            constant = value ? m_constantTrue : m_constantFalse;
        }
    };
    std::visit(visitor, ast->value);
    ast->llvmValue = constant;
}

llvm::Constant* CodeGen::getStringConstant(StringRef str) noexcept {
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

//------------------------------------------------------------------
// Binary Operation
//------------------------------------------------------------------

void CodeGen::visit(AstBinaryExpr* ast) noexcept {
    switch (Token::getOperatorType(ast->tokenKind)) {
    case OperatorType::Arithmetic:
        return arithmetic(ast);
    case OperatorType::Logical:
        return logical(ast);
    case OperatorType::Comparison:
        return comparison(ast);
    }
}

void CodeGen::logical(AstBinaryExpr* ast) noexcept {
    // lhs
    visit(ast->lhs.get());
    auto* lhsBlock = m_builder.GetInsertBlock();

    auto* func = lhsBlock->getParent();
    const auto isAnd = ast->tokenKind == TokenKind::LogicalAnd;
    auto prefix = isAnd ? "and"s : "or"s;
    auto* elseBlock = llvm::BasicBlock::Create(m_llvmContext, prefix, func);
    auto* endBlock = llvm::BasicBlock::Create(m_llvmContext, prefix + ".end", func);

    if (isAnd) {
        m_builder.CreateCondBr(ast->lhs->llvmValue, elseBlock, endBlock);
    } else {
        m_builder.CreateCondBr(ast->lhs->llvmValue, endBlock, elseBlock);
    }

    // rhs
    m_builder.SetInsertPoint(elseBlock);
    visit(ast->rhs.get());
    auto* rhsBlock = m_builder.GetInsertBlock();
    m_builder.CreateBr(endBlock);

    // phi
    endBlock->moveAfter(rhsBlock);
    m_builder.SetInsertPoint(endBlock);
    auto* phi = m_builder.CreatePHI(ast->type->getLlvmType(m_context), 2);

    if (isAnd) {
        phi->addIncoming(m_constantFalse, lhsBlock);
    } else {
        phi->addIncoming(m_constantTrue, lhsBlock);
    }
    phi->addIncoming(ast->rhs->llvmValue, rhsBlock);
    ast->llvmValue = phi;
}

void CodeGen::comparison(AstBinaryExpr* ast) noexcept {
    visit(ast->lhs.get());
    visit(ast->rhs.get());

    const auto* ty = ast->lhs->type;
    llvm::CmpInst::Predicate pred{};
    if (ty->isIntegral()) {
        auto sign = ty->isSignedIntegral();
        switch (ast->tokenKind) {
        case TokenKind::Equal:
            pred = llvm::CmpInst::Predicate::ICMP_EQ;
            break;
        case TokenKind::NotEqual:
            pred = llvm::CmpInst::Predicate::ICMP_NE;
            break;
        case TokenKind::LessThan:
            pred = sign ? llvm::CmpInst::Predicate::ICMP_SLT : llvm::CmpInst::Predicate::ICMP_ULT;
            break;
        case TokenKind::LessOrEqual:
            pred = sign ? llvm::CmpInst::Predicate::ICMP_SLE : llvm::CmpInst::Predicate::ICMP_ULE;
            break;
        case TokenKind::GreaterOrEqual:
            pred = sign ? llvm::CmpInst::Predicate::ICMP_SGE : llvm::CmpInst::Predicate::ICMP_UGE;
            break;
        case TokenKind::GreaterThan:
            pred = sign ? llvm::CmpInst::Predicate::ICMP_SGT : llvm::CmpInst::Predicate::ICMP_UGT;
            break;
        default:
            llvm_unreachable("Unkown comparison op");
        }
    } else if (ty->isFloatingPoint()) {
        switch (ast->tokenKind) {
        case TokenKind::Equal:
            pred = llvm::CmpInst::Predicate::FCMP_OEQ;
            break;
        case TokenKind::NotEqual:
            pred = llvm::CmpInst::Predicate::FCMP_UNE;
            break;
        case TokenKind::LessThan:
            pred = llvm::CmpInst::Predicate::FCMP_OLT;
            break;
        case TokenKind::LessOrEqual:
            pred = llvm::CmpInst::Predicate::FCMP_OLE;
            break;
        case TokenKind::GreaterOrEqual:
            pred = llvm::CmpInst::Predicate::FCMP_OGE;
            break;
        case TokenKind::GreaterThan:
            pred = llvm::CmpInst::Predicate::FCMP_OGT;
            break;
        default:
            llvm_unreachable("Unkown comparison op");
        }
    } else if (ty->isBoolean()) {
        switch (ast->tokenKind) {
        case TokenKind::Equal:
            pred = llvm::CmpInst::Predicate::ICMP_EQ;
            break;
        case TokenKind::NotEqual:
            pred = llvm::CmpInst::Predicate::ICMP_NE;
            break;
        default:
            llvm_unreachable("Unkown comparison op");
        }
    } else {
        llvm_unreachable("Unkown comparison type");
    }
    ast->llvmValue = m_builder.CreateCmp(pred, ast->lhs->llvmValue, ast->rhs->llvmValue);
}

void CodeGen::arithmetic(AstBinaryExpr* ast) noexcept {
    visit(ast->lhs.get());
    visit(ast->rhs.get());

    const auto* ty = ast->lhs->type;
    llvm::Instruction::BinaryOps op{};

    if (ty->isIntegral()) {
        auto sign = ty->isSignedIntegral();
        switch (ast->tokenKind) {
        case TokenKind::Multiply:
            op = llvm::Instruction::Mul;
            break;
        case TokenKind::Divide:
            op = sign ? llvm::Instruction::SDiv : llvm::Instruction::UDiv;
            break;
        case TokenKind::Modulus:
            op = sign ? llvm::Instruction::SRem : llvm::Instruction::URem;
            break;
        case TokenKind::Plus:
            op = llvm::Instruction::Add;
            break;
        case TokenKind::Minus:
            op = llvm::Instruction::Sub;
            break;
        default:
            llvm_unreachable("Unknown binary op");
        }
    } else if (ty->isFloatingPoint()) {
        switch (ast->tokenKind) {
        case TokenKind::Multiply:
            op = llvm::Instruction::FMul;
            break;
        case TokenKind::Divide:
            op = llvm::Instruction::FDiv;
            break;
        case TokenKind::Modulus:
            op = llvm::Instruction::FRem;
            break;
        case TokenKind::Plus:
            op = llvm::Instruction::FAdd;
            break;
        case TokenKind::Minus:
            op = llvm::Instruction::FSub;
            break;
        default:
            llvm_unreachable("Unknown binary op");
        }
    } else {
        llvm_unreachable("Unsupported binary op type");
    }
    ast->llvmValue = m_builder.CreateBinOp(op, ast->lhs->llvmValue, ast->rhs->llvmValue);
}

//------------------------------------------------------------------
// Casting
//------------------------------------------------------------------

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

void CodeGen::visit(AstIfExpr* ast) noexcept {
    visit(ast->expr.get());
    visit(ast->trueExpr.get());
    visit(ast->falseExpr.get());
    ast->llvmValue = m_builder.CreateSelect(
        ast->expr->llvmValue,
        ast->trueExpr->llvmValue,
        ast->falseExpr->llvmValue);
}

unique_ptr<llvm::Module> CodeGen::getModule() noexcept {
    return std::move(m_module);
}
