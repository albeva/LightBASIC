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
  m_llvmContext{ context.getLlvmContext() } {
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

    if (generateMain) {
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

    // parse statements
    visit(ast->stmtList.get());

    // close main
    if (generateMain) {
        auto* retValue = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(m_llvmContext));
        auto& lastBlock = m_function->getBasicBlockList().back();
        llvm::ReturnInst::Create(m_llvmContext, retValue, &lastBlock);
    }

    if ((m_globalCtorBlock != nullptr) && (m_globalCtorBlock->getTerminator() == nullptr)) {
        llvm::ReturnInst::Create(m_llvmContext, nullptr, m_globalCtorBlock);
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
    new llvm::StoreInst(ast->expr->llvmValue, dstValue, m_block);
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
        RESTORE_ON_EXIT(m_block);
        m_block = getGlobalCtorBlock();
        visit(ast->expr.get());
        new llvm::StoreInst(ast->expr->llvmValue, value, m_block);
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

    auto* value = new llvm::AllocaInst(
        exprType,
        0,
        ast->symbol->identifier(),
        m_block);

    if (exprValue != nullptr) {
        new llvm::StoreInst(exprValue, value, m_block);
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
            declareFunc(static_cast<AstFuncDecl*>(stmt.get()));
            break;
        case AstKind::FuncStmt:
            declareFunc(static_cast<AstFuncStmt*>(stmt.get())->decl.get());
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
    RESTORE_ON_EXIT(m_function);
    RESTORE_ON_EXIT(m_scope);
    m_scope = Scope::Function;
    m_function = llvm::cast<llvm::Function>(ast->decl->symbol->getLlvmValue());
    m_block = llvm::BasicBlock::Create(m_llvmContext, "", m_function);

    for (const auto& param : ast->decl->paramDecls) {
        auto* sym = param->symbol;
        auto* value = sym->getLlvmValue();
        sym->setLlvmValue(new llvm::AllocaInst( // NOLINT
            sym->type()->getLlvmType(m_context),
            0,
            sym->identifier() + ".addr",
            m_block));
        new llvm::StoreInst(value, sym->getLlvmValue(), m_block);
    }

    visit(ast->stmtList.get());

    if (m_block->getTerminator() == nullptr) {
        auto* retType = m_function->getReturnType();
        if (!retType->isVoidTy()) {
            fatalError("No RETURN statement");
        }
        llvm::ReturnInst::Create(m_llvmContext, nullptr, m_block);
    }
}

void CodeGen::visit(AstReturnStmt* ast) noexcept {
    if (ast->expr) {
        visit(ast->expr.get());
        llvm::ReturnInst::Create(m_llvmContext, ast->expr->llvmValue, m_block);
    } else {
        llvm::ReturnInst::Create(m_llvmContext, nullptr, m_block);
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
    if (isa<TypeFunction>(type)) {
        ast->llvmValue = ast->symbol->getLlvmValue();
        return;
    }

    auto* sym = ast->symbol;
    ast->llvmValue = new llvm::LoadInst(
        sym->type()->getLlvmType(m_context),
        sym->getLlvmValue(),
        ast->symbol->identifier(),
        m_block);
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

    auto* inst = llvm::CallInst::Create(llvm::FunctionCallee(fn), args, "", m_block);
    inst->setTailCall(false);
    ast->llvmValue = inst;
}

void CodeGen::visit(AstLiteralExpr* ast) noexcept {
    llvm::Constant* constant = nullptr;
    auto visitor = Visitor{
        [&](std::monostate) {
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
                static_cast<const TypeIntegral*>(ast->type)->isSigned());
        },
        [&](double value) {
            constant = llvm::ConstantFP::get(
                ast->type->getLlvmType(m_context),
                value);
        },
        [&](bool value) {
            constant = llvm::ConstantInt::get(
                ast->type->getLlvmType(m_context),
                value,
                static_cast<const TypeBoolean*>(ast->type)->isSigned());
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
            ast->llvmValue = llvm::BinaryOperator::CreateNeg(expr->llvmValue, "", m_block);
        } else if (expr->llvmValue->getType()->isFloatingPointTy()) {
            ast->llvmValue = llvm::UnaryOperator::CreateFNeg(expr->llvmValue, "", m_block);
        }

        break;
    }
    default:
        fatalError("Unsupported unary operator: '"_t + Token::description(ast->tokenKind) + "'");
    }
}

void CodeGen::visit(AstCastExpr* ast) noexcept {
    visit(ast->expr.get());

    bool srcIsSigned = false;
    if (const auto* src = dyn_cast<TypeIntegral>(ast->expr->type)) {
        srcIsSigned = src->isSigned();
    }

    bool dstIsSigned = false;
    if (const auto* dst = dyn_cast<TypeIntegral>(ast->type)) {
        dstIsSigned = dst->isSigned();
    }

    auto opcode = llvm::CastInst::getCastOpcode(
        ast->expr->llvmValue,
        srcIsSigned,
        ast->type->getLlvmType(m_context),
        dstIsSigned);

    auto* value = llvm::CastInst::Create(
        opcode,
        ast->expr->llvmValue,
        ast->type->getLlvmType(m_context),
        "",
        m_block);

    ast->llvmValue = value;
}

unique_ptr<llvm::Module> CodeGen::getModule() noexcept {
    return std::move(m_module);
}
