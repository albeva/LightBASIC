//
// Created by Albert on 05/07/2020.
//
#include "CodeGen.h"
#include "Ast/Ast.h"
#include "Lexer/Token.h"
#include "Symbol/Symbol.h"
#include "Symbol/SymbolTable.h"
#include "Type/Type.h"
#include "llvm/IR/IRPrintingPasses.h"
#include <charconv>
using namespace lbc;

[[noreturn]] static void error(const string& message) {
    std::cerr << message << std::endl;
    std::exit(EXIT_FAILURE);
}

CodeGen::CodeGen(llvm::LLVMContext& context, llvm::SourceMgr& srcMgr, unsigned fileId)
  : m_context{ context },
    m_srcMgr{ srcMgr },
    m_fileId{ fileId },
    m_builder{ context } {}

void CodeGen::visit(AstProgram* ast) {
    auto file = m_srcMgr.getMemoryBuffer(m_fileId)->getBufferIdentifier();

    m_module = make_unique<llvm::Module>(file, m_context);
    m_module->setTargetTriple(llvm::sys::getDefaultTargetTriple());

    m_function = llvm::Function::Create(
        llvm::FunctionType::get(llvm::Type::getVoidTy(m_context), false),
        llvm::Function::ExternalLinkage,
        "main",
        *m_module);
    m_function->setCallingConv(llvm::CallingConv::C);

    m_block = llvm::BasicBlock::Create(m_context, "", m_function);

    ast->stmtList->accept(this);

    llvm::ReturnInst::Create(m_module->getContext(), nullptr, m_block);

    if (llvm::verifyModule(*m_module, &llvm::outs())) {
        std::cerr << "Failed to verify modeul" << '\n';
        std::exit(EXIT_FAILURE);
    }

    auto* printer = llvm::createPrintModulePass(llvm::outs());
    printer->runOnModule(*m_module);
}

void CodeGen::visit(AstStmtList* ast) {
    for (const auto& stmt : ast->stmts) {
        stmt->accept(this);
    }
}

void CodeGen::visit(AstAssignStmt* ast) {
}

void CodeGen::visit(AstExprStmt* ast) {
    ast->expr->accept(this);
}

void CodeGen::visit(AstVarDecl* ast) {
    ast->typeExpr->accept(this);

    bool isConstantArray = false;
    llvm::Constant* exprValue = nullptr;
    if (auto* expr = dyn_cast<AstLiteralExpr>(ast->expr.get())) {
        ast->expr->accept(this);
        exprValue = llvm::cast<llvm::Constant>(ast->expr->llvmValue);
        isConstantArray = expr->token->kind() == TokenKind::StringLiteral;
    }

    if (exprValue == nullptr) {
        error("var decl must have an expr");
    }

    auto* value = new llvm::GlobalVariable(
        *m_module,
        exprValue->getType(),
        false,
        llvm::GlobalValue::PrivateLinkage,
        exprValue);

    if (isConstantArray) {
        llvm::Constant* zero_32 = llvm::Constant::getNullValue(llvm::IntegerType::getInt32Ty(m_context));
        std::array indices{
            zero_32,
            zero_32
        };

        ast->symbol->setValue(llvm::ConstantExpr::getGetElementPtr(nullptr, value, indices, true));
    } else {
        ast->symbol->setValue(value);
    }
}

void CodeGen::visit(AstFuncDecl* ast) {
    auto* fnTy = llvm::cast<llvm::FunctionType>(ast->symbol->type()->llvmType(m_context));
    auto* fn = llvm::Function::Create(
        fnTy,
        llvm::GlobalValue::ExternalLinkage,
        view_to_stringRef(ast->symbol->alias()),
        *m_module);
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
    if (isa<TypeZString>(type) || isa<TypeFunction>(type)) {
        ast->llvmValue = ast->symbol->value();
        return;
    }
    ast->llvmValue = new llvm::LoadInst(ast->symbol->value(), "", m_block);
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
    case TokenKind::StringLiteral:
        constant = llvm::ConstantDataArray::getString(
            m_context,
            view_to_stringRef(lexeme),
            true);
        break;
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
            llvm::cast<llvm::PointerType>(TypeAny::get()->llvmType(m_context)));
        break;
    default:
        error("Invalid literal type");
    }

    ast->llvmValue = constant;
}
