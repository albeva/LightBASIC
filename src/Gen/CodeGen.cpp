//
// Created by Albert on 05/07/2020.
//
#include "CodeGen.h"
#include "Ast/Ast.h"
#include "Lexer/Token.h"
#include "llvm/IR/IRPrintingPasses.h"
using namespace lbc;

[[noreturn]] static void error(const string& message) {
    std::cerr << message << std::endl;
    std::exit(EXIT_FAILURE);
}

CodeGen::CodeGen(llvm::LLVMContext& context)
  : m_context{ context }, m_builder{ context } {}

void CodeGen::visit(AstProgram* ast) {
    m_module = make_unique<llvm::Module>("app", m_context);
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
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(AstExprStmt* ast) {
    ast->expr->accept(this);
}

void CodeGen::visit(AstVarDecl* ast) {
    llvm::Constant* constant = nullptr;
    if (const auto* expr = dyn_cast<AstLiteralExpr>(ast->expr.get())) {
        switch (expr->token->kind()) {
        case TokenKind::StringLiteral:
            constant = llvm::ConstantDataArray::getString(
            m_context,
            view_to_stringRef(expr->token->lexeme()),
            true);
            break;
        default:
            error("Unsupported expression typeExpr");
            break;
        }
    } else {
        error("Unsupported expression typeExpr");
    }

    auto name = string(ast->identExpr->token->lexeme());
    auto* value = new llvm::GlobalVariable(
    *m_module,
    constant->getType(),
    true,
    llvm::GlobalValue::PrivateLinkage,
    constant,
    ".str");
    value->setAlignment(llvm::MaybeAlign(1));
    value->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);

    llvm::Constant* zero_32 = llvm::Constant::getNullValue(llvm::IntegerType::getInt64Ty(m_context));
    std::array indices{
        zero_32,
        zero_32
    };
    m_values[name] = llvm::ConstantExpr::getGetElementPtr(nullptr, value, indices, true);
}

void CodeGen::visit(AstFuncDecl* ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(AstFuncParamDecl* ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(AstAttributeList* ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(AstAttribute* ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(AstIdentExpr* ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(AstTypeExpr* ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}

void CodeGen::visit(AstCallExpr* ast) {
    auto* fn = getOrCreate(ast);
    if (fn == nullptr) {
        error("Unknown function");
    }

    std::vector<llvm::Value*> args;
    args.reserve(ast->argExprs.size());
    for (const auto& arg : ast->argExprs) {
        if (auto* id = dyn_cast<AstIdentExpr>(arg.get())) {
            auto iter = m_values.find(string(id->token->lexeme()));
            if (iter != m_values.end()) {
                args.emplace_back(iter->second);
            } else {
                error("Undefined");
            }
        } else {
            error("Unsupprted");
        }
    }

    auto* inst = llvm::CallInst::Create(llvm::FunctionCallee(fn), args, "", m_block);
    inst->setTailCall(false);
}

llvm::Function* CodeGen::getOrCreate(AstCallExpr* ast) {
    auto name = string(ast->identExpr->token->lexeme());
    if (name == "print") {
        auto iter = m_values.find(name);
        if (iter == m_values.end()) {
            // pretend:
            // int puts(const char*)
            auto* resTy = llvm::IntegerType::get(m_context, 32);
            auto* argBaseTy = llvm::IntegerType::get(m_context, 8);
            auto* argTy = argBaseTy->getPointerTo();
            auto* fnTy = llvm::FunctionType::get(resTy, { argTy }, false);

            auto* fn = llvm::Function::Create(fnTy, llvm::GlobalValue::ExternalLinkage, "puts", *m_module);
            fn->addParamAttr(0, llvm::Attribute::AttrKind::ReadOnly);
            fn->addParamAttr(0, llvm::Attribute::AttrKind::NoCapture);
            fn->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Local);
            fn->setDSOLocal(true);

            m_values[name] = fn;
            return fn;
        }
        return dyn_cast<llvm::Function>(iter->second);
    }
    return nullptr;
}

void CodeGen::visit(AstLiteralExpr* ast) {
    std::cerr << "Not implemented: " << __PRETTY_FUNCTION__ << '\n';
}
