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
  m_llvmContext{ context.getLlvmContext() } {
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
    visitStmtList(ast->stmtList.get());

    // close main
    if (generateMain) {
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
    new llvm::StoreInst(ast->expr->llvmValue, dstValue, m_block);
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
            exprType = ast->expr->type->llvmType(m_context);
            exprValue = llvm::Constant::getNullValue(exprType);
            generateStoreInCtror = true;
        }
    } else {
        exprType = ast->typeExpr->type->llvmType(m_context);
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
        value = new llvm::AllocaInst(
            exprType,
            0,
            ast->symbol->identifier(),
            m_block);
        new llvm::StoreInst(exprValue, value, m_block);
    }

    ast->symbol->setValue(value);
}

void CodeGen::visitFuncDecl(AstFuncDecl* ast) {
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
    auto* fnTy = llvm::cast<llvm::FunctionType>(ast->symbol->type()->llvmType(m_context));
    auto* fn = llvm::Function::Create(
        fnTy,
        llvm::GlobalValue::ExternalLinkage,
        ast->symbol->identifier(),
        *m_module);
    fn->setCallingConv(llvm::CallingConv::C);
    fn->setDSOLocal(true);
    ast->symbol->setValue(fn);

    auto iter = fn->arg_begin();
    for (const auto& param : ast->paramDecls) {
        iter->setName(param->symbol->identifier());
        param->symbol->setValue(iter);
        iter++;
    }
}

void CodeGen::visitFuncParamDecl(AstFuncParamDecl* /*ast*/) {
    llvm_unreachable("visitFuncParamDecl");
}

void CodeGen::visitFuncStmt(AstFuncStmt* ast) {
    RESTORE_ON_EXIT(m_function);
    RESTORE_ON_EXIT(m_scope);
    m_scope = Scope::Function;
    m_function = llvm::cast<llvm::Function>(ast->decl->symbol->value());
    m_block = llvm::BasicBlock::Create(m_llvmContext, "", m_function);

    for (const auto& param : ast->decl->paramDecls) {
        auto* sym = param->symbol;
        auto* value = sym->value();
        sym->setValue(new llvm::AllocaInst(
            sym->type()->llvmType(m_context),
            0,
            sym->identifier() + ".addr",
            m_block));
        new llvm::StoreInst(value, sym->value(), m_block);
    }

    visitStmtList(ast->stmtList.get());

    if (!m_block->getTerminator()) {
        auto* retType = m_function->getReturnType();
        llvm::Value* retValue;
        if (retType->isVoidTy()) {
            retValue = nullptr;
        } else {
            fatalError("No RETURN statement");
        }
        llvm::ReturnInst::Create(m_llvmContext, nullptr, m_block);
    }
}

void CodeGen::visitReturnStmt(AstReturnStmt* ast) {
    if (ast->expr) {
        visitExpr(ast->expr.get());
        llvm::ReturnInst::Create(m_llvmContext, ast->expr->llvmValue, m_block);
    } else {
        llvm::ReturnInst::Create(m_llvmContext, nullptr, m_block);
    }
}

void CodeGen::visitAttributeList(AstAttributeList* /*ast*/) {
    llvm_unreachable("visitAttributeList");
}

void CodeGen::visitAttribute(AstAttribute* /*ast*/) {
    llvm_unreachable("visitAttribute");
}

void CodeGen::visitIdentExpr(AstIdentExpr* ast) {
    const auto* type = ast->type;
    if (isa<TypeFunction>(type)) {
        ast->llvmValue = ast->symbol->value();
        return;
    }

    auto* sym = ast->symbol;
    ast->llvmValue = new llvm::LoadInst(
        sym->type()->llvmType(m_context),
        sym->value(),
        ast->symbol->identifier(),
        m_block);
}

void CodeGen::visitTypeExpr(AstTypeExpr* /*ast*/) {
    llvm_unreachable("visitTypeExpr");
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
    ast->llvmValue = inst;
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
            m_stringLiterals.insert({ lexeme, constant });
        }
        break;
    }
    case TokenKind::NumberLiteral: {
        uint64_t result = 0;
        std::from_chars(lexeme.data(), lexeme.data() + lexeme.size(), result); // NOLINT
        constant = llvm::ConstantInt::get(
            ast->type->llvmType(m_context),
            result,
            llvm::cast<TypeNumeric>(ast->type)->isSigned());
        break;
    }
    case TokenKind::BooleanLiteral: {
        uint64_t value = lexeme == "TRUE" ? 1 : 0;
        constant = llvm::ConstantInt::get(
            ast->type->llvmType(m_context),
            value,
            llvm::cast<TypeNumeric>(ast->type)->isSigned());
        break;
    }
    case TokenKind::NullLiteral:
        constant = llvm::ConstantPointerNull::get(
            llvm::cast<llvm::PointerType>(ast->type->llvmType(m_context)));
        break;
    default:
        fatalError("Invalid literal type");
    }

    ast->llvmValue = constant;
}

unique_ptr<llvm::Module> CodeGen::getModule() {
    return std::move(m_module);
}
